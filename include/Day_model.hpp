#pragma once

#include<QObject>
#include <QAbstractListModel>
#include <QAbstractItemModel>
#include <QTimer>
#include <QtQml/qqmlregistration.h>
#include "bserver.hpp"
#include "bclient.hpp"
#include"Hour_model.hpp"

class Day_box : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QDate day READ day WRITE set_day NOTIFY day_changed)
    Q_PROPERTY(Hour_model* hour_model  READ hour_model WRITE set_hour_model NOTIFY hour_model_changed)

public:
    QDate day(void){return day_m;}
    void set_day(QDate day_){day_m=day_;emit day_changed();}

    Hour_model* hour_model(void)const{return hour_model_m;}
    void set_hour_model(Hour_model* ptr){hour_model_m=ptr;emit hour_model_changed();}

    Day_box(QDate day_,Hour_model * hour_model_,QObject *parent):QObject(parent),day_m(day_),hour_model_m(hour_model_){
        hour_model_->setParent(this);

    };

signals:
    void day_changed(void);
    void hour_model_changed(void);

private:
    Hour_model * hour_model_m;
    QDate day_m;

};

class Day_model : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count CONSTANT)
    Q_PROPERTY(int total_selected  READ total_selected  NOTIFY total_selected_changed)
    Q_PROPERTY(Book_Server* book_server  READ book_server  WRITE set_book_server NOTIFY book_server_changed)
    Q_PROPERTY(Book_Client* book_client  READ book_client  WRITE set_book_client NOTIFY book_client_changed)

public:
    enum ModelRoles {
        dayRole = Qt::UserRole + 1,
        hour_modelRole,selectedRole
    };
    int count() const;
    void add_to_total_selected(int sel){total_selected_+=sel; emit total_selected_changed(total_selected_);}
    Book_Server* book_server(void)const{return book_server_;}
    void set_book_server(Book_Server* ptr){book_server_=ptr; emit book_server_changed();}
    int total_selected(void)const{return total_selected_;}

    Book_Client* book_client(void)const{return book_client_;}
    void set_book_client(Book_Client* ptr){book_client_=ptr; emit book_client_changed();}

    explicit Day_model(QObject *parent = nullptr);
    Q_INVOKABLE void get_new_bookings(void);
    void add_booking(const Booking nbook);
    void add_sent_booking(const Booking);
    void remove_sent_booking(const Booking);

    void append(Day_box* o);
    void pop_front(void);
    void update_list(void);

    int rowCount(const QModelIndex &p) const;
    QVariant data(const QModelIndex &index, int role) const;

    QHash<int, QByteArray> roleNames() const;



signals:
    void countChanged(int count);
    void total_selected_changed(int sele);
    void new_bookings(Booking nbook );
    void book_server_changed(void);
    void book_client_changed(void);




private:
    int m_count;
    QList<Day_box*> m_days;
    QTimer *timer_m;
    Book_Server* book_server_;
    Book_Client* book_client_;
    int total_selected_;

};

