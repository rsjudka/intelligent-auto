#include <fcntl.h>
#include <unistd.h>

#include <algorithm>
#include <iostream>
#include <vector>

#include <obd/obd.hpp>

OBD::OBD()
{
    btSocket = new QBluetoothSocket(QBluetoothServiceInfo::RfcommProtocol);
    QObject::connect(btSocket, &QBluetoothSocket::connected, this, &OBD::btConnected);
    QObject::connect(btSocket, &QBluetoothSocket::stateChanged, this, &OBD::socketChanged);
    btSocket->connectToService(QBluetoothAddress(QString("00:1D:A5:68:98:8C")), QBluetoothUuid(QString("00001101-0000-1000-8000-00805F9B34FB")), QIODevice::ReadWrite);
    // this->connect("/dev/ttyUSB0", B115200);
    // if (this->connected) this->initialize();
}

OBD::~OBD()
{
    this->raw_query("ATZ");
    sleep(1);
    close(this->fd);
    this->connected = false;
}


void OBD::btConnected(){
    this->adapterType = BT; 
    this->connected=true;
    this->initialize();
}

void OBD::socketChanged(QBluetoothSocket::SocketState state){
        switch(state){
            case(QBluetoothSocket::UnconnectedState):
                std::cout<<"[IntelligentAuto][OBD BT] Unconnected"<<std::endl;
                break;
            case(QBluetoothSocket::ConnectingState):
                std::cout<<"[IntelligentAuto][OBD BT] Attempting connection"<<std::endl;
                break;
            case(QBluetoothSocket::ConnectedState):
                std::cout<<"[IntelligentAuto][OBD BT] Connected"<<std::endl;
                break;
            case(QBluetoothSocket::ServiceLookupState):
                std::cout<<"[IntelligentAuto][OBD BT] Looking up services"<<std::endl;
                break;
            default:
                std::cout<<"[IntelligentAuto][OBD BT] unimplemented"<<std::endl;

        }
}


bool OBD::query(Command &cmd, double &val)
{
    std::cout<<"[IntelligentAuto][OBD BT] QUERY"<<std::endl;
    if (!this->connected) return false;

    std::lock_guard<std::mutex> guard(this->obd_mutex);
    if (this->send(cmd.request)) {
        Response resp = this->receive();
        val = cmd.decoder(resp);
        return resp.success;
    }
    else {
        std::cout << "[IntelligentAuto][OBD BT] unable to send command" << std::endl;
    }

    return false;
}

OBD *OBD::get_instance()
{
    static OBD obd;
    return &obd;
}

void OBD::connect(std::string dev_path, speed_t baudrate)
{
    if (this->connected) return;

    this->fd = open(dev_path.c_str(), O_RDWR | O_NOCTTY | O_NDELAY);
    if (this->fd == -1) {
        this->connected = false;
        return;
    }

    fcntl(this->fd, F_SETFL, 0);

    struct termios options;
    tcgetattr(this->fd, &options);

    options.c_cflag |= (CLOCAL | CREAD);
    options.c_lflag &= !(ICANON | ECHO | ECHOE | ISIG);
    options.c_oflag &= !(OPOST);
    options.c_cc[VMIN] = 0;
    options.c_cc[VTIME] = 100;

    tcflush(this->fd, TCIOFLUSH);
    tcsetattr(this->fd, TCSANOW, &options);

    cfsetispeed(&options, baudrate);
    cfsetospeed(&options, baudrate);
    tcsetattr(this->fd, TCSANOW, &options);
    tcflush(this->fd, TCIOFLUSH);

    this->connected = true;
}

void OBD::initialize()
{
    std::vector<std::string> cmds = {"ATD", "ATZ", "ATE0", "ATL0", "ATS0", "ATH0", "ATSP0", "0100"};
    for (auto const cmd : cmds) {
        this->raw_query(cmd);
        usleep(500000);
    }
}

bool OBD::send(Request req)
{
    std::string req_str = req.to_str();

    return this->_write(req_str) >= 0;
}

int OBD::_write(std::string str)
{
    str += '\r';
    int size;
    if ((size = (adapterType==BT)?(::send(btSocket->socketDescriptor(), str.c_str(), str.length(),0)):(write(this->fd, str.c_str(), str.length()))) < 0) {
        std::cout << "[IntelligentAuto][OBD BT] failed write" << std::endl;
        this->connected = false;
        return 0;
    }
    std::cout<<"[IntelligentAuto][OBD BT] Wrote "<<str.c_str()<<" with "<<size<<" bytes"<<std::endl;

    return size;
}

bool OBD::is_failed_response(std::string str)
{
    std::vector<std::string> failed_msgs = {
        "UNABLE TO CONNECT", "BUS INIT...ERROR", "NO DATA", "STOPPED", "ERROR", "?"};

    for (auto const &msg : failed_msgs)
        if (str.find(msg) != std::string::npos) return true;

    return false;
}

Response OBD::receive()
{
    std::string resp_str = this->_read();

    if (is_failed_response(resp_str)) return Response();

    std::string searching_phrase = "SEARCHING...";
    std::string::size_type i = resp_str.find(searching_phrase);
    if (i != std::string::npos) resp_str.erase(i, searching_phrase.length());
    resp_str.erase(
        std::remove_if(resp_str.begin(), resp_str.end(), [](auto const &c) -> bool { return !std::isalnum(c); }),
        resp_str.end());

    return Response(resp_str);
}

std::string OBD::_read()
{
    char buf[1];
    std::string str;
  
    while (true) {
        if (((adapterType==BT)?(::recv(btSocket->socketDescriptor(), (void *) buf, 1, 0)):(read(this->fd, (void *)buf, 1))) == -1) {
            std::cout << "failed read" << std::endl;
            if(adapterType != BT){
                this->connected = false;
                return "";
            }else{
                usleep(500000); //bt is a little slower than USB in this case it seems, give it a little rest
                //TODO: actually make a case for failing read on BT resulting in a disconnect (probably just rely on BT state instead of reading?)
                continue;
            }
        }
        //std::cout<<"[IntelligentAuto][OBD BT] Read "<<buf[0]<< " from "<<((adapterType==BT)?("bluetooth"):("usb"))<<std::endl;
        if (buf[0] == '>')
            break;
        else
            str += buf[0];
    }
    std::cout<<"[IntelligentAuto][OBD BT] Read "<<str<< " from "<<((adapterType==BT)?("bluetooth"):("usb"))<<std::endl;
    return str;
}
