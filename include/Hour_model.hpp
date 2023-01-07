#pragma once

#include <QAbstractListModel>
#include<QObject>
#include <QtQml/qqmlregistration.h>
#include <QAbstractItemModel>
#include"booking.hpp"
class Book_Obj :public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString code_str READ code_str CONSTANT)
    Q_PROPERTY(QDateTime start READ start CONSTANT)
    Q_PROPERTY(QDateTime finish READ finish CONSTANT)
    QML_ELEMENT
    QML_SINGLETON

public:
    using QObject::QObject;
    Book_Obj(Booking book_m,QObject *parent):QObject(parent),book_(book_m){};
    QString code_str(void)const{return book_.code_str();}
    QDateTime start(void)const{return book_.start();}
    QDateTime finish(void)const{return book_.finish();}

private:
    Booking book_;
};

class Hour_box : public QObject
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(QString hour READ hour CONSTANT)
    Q_PROPERTY(bool booked READ booked WRITE set_booked NOTIFY booked_changed)
    Q_PROPERTY(bool selected READ selected WRITE set_selected NOTIFY selected_changed)
    Q_PROPERTY(bool sentbook READ sentbook WRITE set_sentbook NOTIFY sentbook_changed)
    Q_PROPERTY(QString code_str READ code_str  WRITE set_code_str NOTIFY code_str_changed)
    Q_PROPERTY(QDateTime start READ start  WRITE set_start NOTIFY start_changed)
    Q_PROPERTY(QDateTime finish READ finish  WRITE set_finish NOTIFY finish_changed)


public:
    Hour_box(QString hour_,bool booked_,bool selected_,int hour_m,QObject *parent):QObject(parent),hour_m(hour_),
        booked_m(booked_),selected_m(selected_),hour_(hour_m),sentbook_m(false),start_(QDateTime::currentDateTime()),
        finish_(QDateTime::currentDateTime()),code_str_("")
    {};
    const int hour_;
    QString hour() const{return hour_m;}
    bool booked() const{return booked_m;}
    bool selected() const{return selected_m;}
    bool sentbook() const{return sentbook_m;}
    QString code_str()const{return code_str_;}
    QDateTime start()const{return start_;}
    QDateTime finish()const{return finish_;}


    void set_booked(bool booked_){if(booked_!=booked_m){booked_m=booked_;emit booked_changed();}};
    void set_selected(bool selected_){if(selected_!=selected_m){selected_m=selected_;emit selected_changed();}};
    void set_sentbook(bool sentbook_){if(sentbook_!=sentbook_m){sentbook_m=sentbook_;emit sentbook_changed();}};
    void set_code_str(QString code_str_m){if(code_str_m!=code_str_){code_str_=code_str_m;emit code_str_changed();}};
    void set_start(QDateTime start_m){if(start_m!=start_){start_=start_m;emit start_changed();}};
    void set_finish(QDateTime finish_m){if(finish_m!=finish_){finish_=finish_m;emit finish_changed();}};

signals:
void booked_changed(void);
void selected_changed(void);
void sentbook_changed(void);
void code_str_changed(void);
void start_changed(void);
void finish_changed(void);

private:
    QString hour_m;
    bool booked_m,selected_m,sentbook_m;
    QDateTime start_,finish_;
    QString code_str_;

};

class Hour_model : public QAbstractListModel
{
    Q_OBJECT
    QML_ELEMENT
    Q_PROPERTY(int count READ count NOTIFY countChanged)


public:
    enum ModelRoles {
        hourRole = Qt::UserRole + 1,
        bookedRole,selectedRole,sentbookRole,code_strRole,startRole,finishRole
    };
    int count() const;
    explicit Hour_model(int hstart,QObject *parent = nullptr);
    Q_INVOKABLE bool setProperty(int i, QString role, const QVariant value);
    void add_booked_hours(const std::vector<int>& bhours);
    void add_sent_booked_hours(const Booking& nbook,const std::vector<int>& booked_hours);
    void rm_sent_booked_hours(const Booking& nbook,const std::vector<int>& booked_hours);
    std::vector<Booking> get_bookings_from_selected(QDate day);

    void pop_front(void);
    void update_list(void);

    int rowCount(const QModelIndex &p) const;
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole);
    QModelIndex index(int row, int column = 0, const QModelIndex &parent = QModelIndex()) const;
    QHash<int, QByteArray> roleNames() const;

signals:
    void countChanged(int count);
    void total_selected_changed(int selecteds);

private:
    int m_count;
    QList<Hour_box*> m_hours;
    Book_Obj* book_;
};

