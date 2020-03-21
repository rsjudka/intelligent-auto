#ifndef OBD_HPP_
#define OBD_HPP_

#include <termios.h>

#include <mutex>
#include <string>

#include <obd/command.hpp>
#include <QBluetoothAddress>
#include <iostream>
#include <QBluetoothSocket>
#include <sys/socket.h>
class OBD : public QObject{
   Q_OBJECT
   public:
    OBD();
    ~OBD();

    bool query(Command &cmd, double &val);

    inline bool is_connected() { return this->connected; }

    static OBD *get_instance();
    enum OBDType {USB, BT};
    inline OBDType get_adapter_type(){return this->adapterType;}
   private:
    void connect(std::string dev_path, speed_t baudrate);
    void initialize();
    bool send(Request req);
    int _write(std::string str);
    bool is_failed_response(std::string str);
    Response receive();
    std::string _read();

    inline std::string raw_query(std::string cmd) { return (this->_write(cmd) > 0) ? this->_read() : ""; }

    int fd;
    std::mutex obd_mutex;
    OBDType adapterType;
    QBluetoothSocket* btSocket;


    bool connected = false;
    public slots:
        void btConnected();
        void socketChanged(QBluetoothSocket::SocketState state);
};

#endif
