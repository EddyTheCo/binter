#pragma once

#include<QObject>
#include<QString>
#include <QtQml/qqmlregistration.h>
#include<booking.hpp>
#include<account.hpp>
#include"client/qclient.hpp"
#include"client/qclientMQTT.hpp"


class Book_Client : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString  server_id READ get_server_id WRITE set_server_id NOTIFY server_id_changed)
    Q_PROPERTY(QString  payment_addr READ get_payment_addr NOTIFY payment_addr_changed)
    Q_PROPERTY(QString  code_str READ get_code_str WRITE set_code_str NOTIFY code_str_changed)
    Q_PROPERTY(quint64  price_per_hour READ get_price_per_hour  NOTIFY price_per_hour_changed)
    Q_PROPERTY(quint64  topay READ get_topay NOTIFY topay_changed)
    Q_PROPERTY(bool  conected READ conected NOTIFY conected_changed)
    Q_PROPERTY(Stte  status READ status WRITE set_status NOTIFY status_changed)
    QML_ELEMENT
    QML_SINGLETON

public:
    Book_Client();

    enum Stte {
            Null,
            Ready,
        };

    Q_ENUM(Stte)
    Stte status(void)const{return status_;}
    void set_status(Stte st){if(st!=status_){status_=st;emit status_changed();}}
    QString get_payment_addr(void)const{return account_.get_payment_addr().addr;}
    void set_server_id(QString server_id_m){server_id_=server_id_m;emit server_id_changed(); qDebug()<<"server_id:"<<server_id_;}
    QString get_server_id(void)const{return server_id_;}
    QString get_code_str(void)const{return code_str_;}
    quint64 get_price_per_hour(void)const{return price_per_hour_;}
    void set_price_per_hour(quint64 price){if(price_per_hour_!=price){price_per_hour_=price;emit price_per_hour_changed(price);}}
    quint64 get_topay(void)const{return topay_;}
    quint64 get_deposit(void)const{return deposit_;}
    void set_code_str(QString code_str_m){code_str_=code_str_m;emit code_str_changed();}
    bool conected(void)const{return conected_;}
    void set_conected(bool con){if(con!=conected_){conected_=con;emit conected_changed();}}

    void try_to_book(const Booking &booking);


signals:
    void payment_addr_changed(void);
    void server_id_changed(void);
    void price_per_hour_changed(quint64 price );
    void code_str_changed();
    void topay_changed();
    void sent_book(Booking);
    void removed_expired(Booking);
    void got_new_saldo(void);
    void got_new_booking(Booking book);
    void conected_changed(void);
    void status_changed(void);

private:
    Booking get_new_booking_from_metadata(qblocks::fl_array<quint16> metadata)const;
    void add_to_pay(quint64 price){topay_+=price;emit topay_changed();}
    void set_to_pay(quint64 price){if(price!=topay_){topay_=price;emit topay_changed();}}
    void monitor_state(void);
    void subscribe_state(void);
    void monitor_saldo(void);
    void check_if_expired(void);
    void collect_storage_deposit(const quint64 ret_amount, const std::shared_ptr<qblocks::Address> ret_address, const Node_output &node_output_)const;
    void check_state_output(const Node_output &node_output_);
    void add_saldo_output(const Node_output &node_output_);
    std::vector<Booking> deserialize_state(QByteArray state);
    qblocks::fl_array<quint16> create_new_bookings_metadata(Booking book)const;
    account account_;
    qiota::Client* iota_client_;
    qiota::ClientMqtt* iota_client_mqtt_;
    QString server_id_,code_str_;
    QByteArray pay_to_address_hash;
    qiota::ResponseMqtt* monitoring_addres;
    quint64 price_per_hour_,topay_,deposit_;
    std::vector<Booking> books_;

    bool conected_;
    Stte status_;

};



