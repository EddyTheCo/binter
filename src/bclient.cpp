#include"bclient.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include<QJsonDocument>
#include<QTimer>

using namespace qiota::qblocks;

using namespace qiota;
using namespace qcrypto;


Book_Client::Book_Client():iota_client_(new qiota::Client(QUrl("https://api.testnet.shimmer.network"))),
    iota_client_mqtt_(new qiota::ClientMqtt(QUrl("wss://api.testnet.shimmer.network:443/api/mqtt/v1"))),account_(account()),
    server_id_(""),pay_to_address_hash(QByteArray(32,0)),conected_(false),topay_(0),status_(Ready)
{

    qDebug()<<"payment addre:"<<account_.get_payment_addr().addr;

    QObject::connect(this,&Book_Client::server_id_changed,this,&Book_Client::monitor_state);
    monitor_saldo();
    QObject::connect(this,&Book_Client::got_new_saldo,this,[=](){set_status(Ready);});

    connect(iota_client_,&qiota::Client::last_blockid,[](qblocks::c_array id)
    {
        qDebug()<<"id:"<<id.toHexString();
    });

}
std::vector<Booking> Book_Client::deserialize_state(QByteArray state)
{
    std::vector<Booking> var;
    qDebug()<<"state.size:"<<state.size();
    if(state.size()>40)
    {
        auto buffer=QDataStream(&state,QIODevice::ReadOnly);
        buffer.setByteOrder(QDataStream::LittleEndian);
        quint16 nbooks;
        buffer>>nbooks;
        qDebug()<<"nbooks:"<<nbooks;
        if(state.size()==42+nbooks*16)
        {
            for (auto i=0;i<nbooks;i++)
            {
                const auto b=Booking(buffer,0);
                if(b.finish()>QDateTime::currentDateTime()&&b.finish().date()<QDate::currentDate().addDays(7))
                {
                    var.push_back(b);
                }


            }
            quint64 price;
            buffer>>price;
            set_price_per_hour(price);
            set_conected(true);

            qDebug()<<"price_perhour:"<<price_per_hour_;
            buffer.readRawData(pay_to_address_hash.data(),32);
            qDebug()<<"pay_to_address_hash:"<<pay_to_address_hash.toHex();

        }
    }


    return var;
}
void Book_Client::check_state_output(const Node_output& node_output_)
{
    if(node_output_.output()->type_m==3)
    {
        const auto basic_output_=*(std::dynamic_pointer_cast<qblocks::Basic_Output>(node_output_.output()));
        const auto metfeau=basic_output_.get_feature_(2);
        const auto tagfeau=basic_output_.get_feature_(3);
        const auto sendfeau=basic_output_.get_feature_(0);
        if(sendfeau)
        {

            const auto sfe=(std::dynamic_pointer_cast<qblocks::Sender_Feature>(sendfeau));
            if(sfe->sender()->type_m==0)
            {

                const auto sfeaddr=(std::dynamic_pointer_cast<qblocks::Ed25519_Address>(sfe->sender()));


                const auto server_hash=qencoding::qbech32::Iota::decode(server_id_).second.right(32);

                if(sfeaddr->pubkeyhash()==server_hash)
                {
                    if(tagfeau)
                    {

                        const auto tfe=(std::dynamic_pointer_cast<qblocks::Tag_Feature>(tagfeau));

                        if(tfe->tag()==fl_array<quint8>("state"))
                        {
                            if(metfeau)
                            {

                                auto metadata_feature=std::dynamic_pointer_cast<qblocks::Metadata_Feature>(metfeau);
                                auto metadata=metadata_feature->data();
                                auto booking_array=deserialize_state(metadata);

                                for(const auto& new_booking:booking_array)
                                {
                                    emit got_new_booking(new_booking);
                                }

                            }
                        }
                    }
                }

            }
        }


    }
}

void Book_Client::subscribe_state(void)
{
    if(iota_client_mqtt_->state()==QMqttClient::Connected)
    {
        qDebug()<<"mqtt subscribed to :"<<"address/"+server_id_;
        auto resp=iota_client_mqtt_->
                get_outputs_unlock_condition_address("address/"+server_id_);
        QObject::connect(resp,&ResponseMqtt::returned,this,[=](QJsonValue data){
            const auto node_outputs_2=Node_output(data);
            check_state_output(node_outputs_2);
        });
    }
}
void Book_Client::monitor_state(void)
{

    if(qencoding::qbech32::Iota::decode(server_id_).second.size())
    {
        auto node_outputs_=new Node_outputs();
        iota_client_->get_basic_outputs(node_outputs_,"address="+server_id_+"&hasStorageDepositReturn=false&hasTimelock=false&hasExpiration=false&sender="
                                        +server_id_+"&tag="+fl_array<quint8>("state").toHexString());
        QObject::connect(node_outputs_,&Node_outputs::finished,this,[=]( ){
            if(node_outputs_->outs_.size())
            {
                check_state_output(node_outputs_->outs_.front());
                node_outputs_->deleteLater();
            }

        });

        QObject::connect(iota_client_mqtt_,&QMqttClient::stateChanged,this,[=](QMqttClient::ClientState state ){
            subscribe_state();
        });
        subscribe_state();
    }
}
void Book_Client::monitor_saldo(void)
{

    QObject::connect(iota_client_mqtt_,&QMqttClient::stateChanged,this,[=](QMqttClient::ClientState state ){
        if(iota_client_mqtt_->state()==QMqttClient::Connected)
        {
            auto resp=iota_client_mqtt_->
                    get_outputs_unlock_condition_address("address/"+account_.get_payment_addr().addr);
            QObject::connect(resp,&ResponseMqtt::returned,this,[=](QJsonValue data){
                const auto node_outputs_2=Node_output(data);
                add_saldo_output(node_outputs_2);
            });
        }
    });

}

qblocks::fl_array<quint16> Book_Client::create_new_bookings_metadata(Booking book)const
{

    qblocks::fl_array<quint16> var;
    auto buffer=QDataStream(&var,QIODevice::WriteOnly | QIODevice::Append);
    buffer.setByteOrder(QDataStream::LittleEndian);

    book.set_code_str(code_str_);

    book.serialize(buffer,1);
    return var;
}
void Book_Client::try_to_book(const Booking& book)
{
    qDebug()<<"try to book";

    auto info=iota_client_->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,iota_client_,[=]( ){

        check_if_expired();
        auto node_outputs_expired_=new Node_outputs();
        auto now=QDateTime::currentDateTime().toSecsSinceEpoch();
        iota_client_->get_basic_outputs(node_outputs_expired_,"expirationReturnAddress="+account_.get_payment_addr().addr+
                                        "&hasExpiration=true"
                                        +"&expiresBefore="+QString::number(now));
        QObject::connect(node_outputs_expired_,&Node_outputs::finished,iota_client_,[=]( ){
            auto node_outputs_=new Node_outputs();
            iota_client_->get_basic_outputs(node_outputs_,"address="+account_.get_payment_addr().addr);

            QObject::connect(node_outputs_,&Node_outputs::finished,iota_client_,[=]( ){

                auto payment_bundle=account_.get_payment_addr();

                auto metadata_=create_new_bookings_metadata(book);
                auto metFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(metadata_));

                auto price=book.calculate_price(price_per_hour_);
                qDebug()<<"price of booking:"<<price;
                auto retAddr=std::shared_ptr<Address>(new Ed25519_Address(account_.get_payment_addr().addr_hash));
                auto sendFea=std::shared_ptr<qblocks::Feature>(new Sender_Feature(retAddr));
                auto var_storageaddrUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(
                            new Storage_Deposit_Return_Unlock_Condition(retAddr,0));

                auto until_when=QDateTime::currentDateTime().addSecs(5*60).toSecsSinceEpoch();
                QTimer::singleShot(450000, this, &Book_Client::check_if_expired);
                auto expirationUnlock=std::shared_ptr<qblocks::Unlock_Condition>
                        (new Expiration_Unlock_Condition(until_when,retAddr));

                auto pay_to_address=std::shared_ptr<Address>(new Ed25519_Address(pay_to_address_hash));


                auto pay_toaddrUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(pay_to_address));

                auto min_deposit_pay_to= Basic_Output(0,{pay_toaddrUnlcon,var_storageaddrUnlcon,expirationUnlock},{sendFea,metFea},{})
                        .min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);

                c_array Inputs_Commitments;
                quint64 amount=0;
                std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
                std::vector<std::shared_ptr<qblocks::Input>> inputs;

                auto outs_v=node_outputs_expired_->outs_;
                std::move((node_outputs_->outs_).begin(), (node_outputs_->outs_).end(), std::back_inserter(outs_v));

                payment_bundle.consume_outputs(outs_v,price+min_deposit_pay_to,Inputs_Commitments,amount,
                                               ret_outputs,inputs);
                qDebug()<<"min_deposit:"<<min_deposit_pay_to;
                qDebug()<<"price:"<<price;
                qDebug()<<"amount:"<<amount;
                if(amount>=price+min_deposit_pay_to)
                {

                    auto storageaddrUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(
                                new Storage_Deposit_Return_Unlock_Condition(retAddr,amount-price));
                    auto Inputs_Commitment=c_array(QCryptographicHash::hash(Inputs_Commitments, QCryptographicHash::Blake2b_256));
                    auto BaOut= std::shared_ptr<qblocks::Output>
                            (new Basic_Output(amount,{pay_toaddrUnlcon,storageaddrUnlcon,expirationUnlock},{sendFea,metFea},{}));

                    ret_outputs.push_back(BaOut);
                    auto essence=std::shared_ptr<qblocks::Essence>
                            (new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,ret_outputs,nullptr));

                    c_array serializedEssence;
                    serializedEssence.from_object<Essence>(*essence);

                    auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
                    std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;
                    payment_bundle.create_unlocks(essence_hash,unlocks);

                    auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,unlocks));


                    auto block_=Block(trpay);


                    iota_client_->send_block(block_);
                    auto copybook=book;

                    copybook.set_code_str(code_str_);

                    books_.push_back(copybook);

                    emit sent_book(copybook);

                    emit topay_changed();
                    set_to_pay(0);
                }
                else
                {
                    set_to_pay(price+min_deposit_pay_to-amount);
                    set_status(Null);
                    emit status_changed();
                    qDebug()<<"Nosendingblock";
                }
                info->deleteLater();
                node_outputs_->deleteLater();
                node_outputs_expired_->deleteLater();
            });

        });

    });

}
void Book_Client::add_saldo_output(const Node_output& v)
{
    if((!v.metadata().is_spent_)&&(v.output()->type_m==3))
    {
        qDebug()<<"got new saldo";
        const auto basic_output_=*(std::dynamic_pointer_cast<qblocks::Basic_Output>(v.output()));
        if(basic_output_.get_unlock_(2))return;

        emit got_new_saldo();

    }

}
void Book_Client::check_if_expired(void)
{
    qDebug()<<"collect_expired";

    auto node_outputs_=new Node_outputs();
    auto now=QDateTime::currentDateTime().toSecsSinceEpoch();
    iota_client_->get_basic_outputs(node_outputs_,"expirationReturnAddress="+account_.get_payment_addr().addr+
                                    "&hasExpiration=true"
                                    +"&expiresBefore="+QString::number(now)+"&sender="
                                    +account_.get_payment_addr().addr);

    qDebug()<<"expirationReturnAddress="+account_.get_payment_addr().addr+
              "&hasExpiration=true"
              +"&expiresBefore="+QString::number(now)+"&sender="
              +account_.get_payment_addr().addr;

    QObject::connect(node_outputs_,&Node_outputs::finished,iota_client_,[=]( ){

        for(const auto& out: node_outputs_->outs_)
        {
            const auto basic_output_=std::dynamic_pointer_cast<qblocks::Basic_Output>(out.output());
            const auto metfeature=basic_output_->get_feature_(2);

            if(metfeature)
            {
                auto metadata_feature=std::dynamic_pointer_cast<qblocks::Metadata_Feature>(metfeature);
                auto metadata=metadata_feature->data();
                auto expired_book=get_new_booking_from_metadata(metadata);
                auto it=std::find_if(books_.begin(),books_.end(),[=](const auto& book_pair){
                    return (book_pair.start()==expired_book.start()&&book_pair.finish()==expired_book.finish());
                });
                if(it!=books_.end())
                {
                    emit removed_expired(expired_book);
                    books_.erase(it);
                }
            }

        }
    });


}
Booking Book_Client::get_new_booking_from_metadata(qblocks::fl_array<quint16> metadata)const
{
    qDebug()<<"metadata.size:"<<metadata.size();
    if(metadata.size()==48)
    {
        auto buffer=QDataStream(&metadata,QIODevice::ReadOnly);
        buffer.setByteOrder(QDataStream::LittleEndian);
        return Booking(buffer,1);
    }
    return Booking(QDateTime::fromMSecsSinceEpoch(0),QDateTime::fromMSecsSinceEpoch(0));
}
