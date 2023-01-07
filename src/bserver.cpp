#include"bserver.hpp"
#include <QCryptographicHash>
#include <QDebug>
#include<QJsonDocument>

using namespace qiota::qblocks;

using namespace qiota;
using namespace qcrypto;
void Book_Server::init_min_amount_funds(void)
{
    auto info=iota_client_->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,iota_client_,[=]( ){
        auto pubOut=get_publish_output(0);
        const auto min_output_pub=std::dynamic_pointer_cast<qblocks::Basic_Output>
                (pubOut)->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);

        auto Addr=std::shared_ptr<Address>(new Ed25519_Address(account_.get_publish_addr().addr_hash));
        auto addrUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(Addr));
        const auto min_deposit_pay= Basic_Output(0,{addrUnlcon},{},{})
                .min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);

        min_amount_funds_=10*min_output_pub+2*min_deposit_pay;
        set_transfer_funds(min_amount_funds_);
    });
}

Book_Server::Book_Server():iota_client_(new qiota::Client(QUrl("https://api.testnet.shimmer.network"))),
    iota_client_mqtt_(new qiota::ClientMqtt(QUrl("wss://api.testnet.shimmer.network:443/api/mqtt/v1"))),account_(account()),
    price_per_hour_(10000),publishing_(false),consolidation_index(1),cons_step_index(0)
{
    init_min_amount_funds();

    connect(this,&Book_Server::state_changed,this,&Book_Server::publish_state);
    connect(iota_client_mqtt_,&QMqttClient::stateChanged,this,[=](QMqttClient::ClientState state ){
        if(iota_client_mqtt_->state()==QMqttClient::Connected)
        {
            auto resp=iota_client_mqtt_->
                    get_outputs_unlock_condition_address("address/"+account_.get_payment_addr(0).addr);
            QObject::connect(resp,&ResponseMqtt::returned,this,&Book_Server::handle_new_book);

            auto resp2=iota_client_mqtt_->
                    get_outputs_unlock_condition_address("address/"+account_.get_publish_addr().addr);
            QObject::connect(resp2,&ResponseMqtt::returned,this,[=](QJsonValue data)
            {
                this->handle_init_funds(data,resp2);
            });
        }

    });
    connect(iota_client_,&qiota::Client::last_blockid,this,[](qblocks::c_array id)
    {
        qDebug()<<id.toHexString();
    });
}
void Book_Server::clean_state(void)
{
    for(const auto& v: books_)
    {
        if(!v.check_validity(QDateTime::currentDateTime()))
        {
            books_.erase(books_.find(v));
        }
    }
}
qblocks::fl_array<quint16> Book_Server::get_state_metadata(void)const
{
    qblocks::fl_array<quint16> var;
    auto buffer=QDataStream(&var,QIODevice::WriteOnly | QIODevice::Append);
    buffer.setByteOrder(QDataStream::LittleEndian);
    buffer<<static_cast<quint16>(books_.size());
    for(auto& v:books_)
    {
        v.serialize(buffer,0);
    }
    buffer<<price_per_hour_;
    buffer.writeRawData(account_.get_payment_addr(0).addr_hash.data(),32);

    return var;

}
bool Book_Server::check_unlock_conditions(const Node_output &output)
{
    qDebug()<<"check_unlock_conditions";
    const auto basic_output_=*(std::dynamic_pointer_cast<qblocks::Basic_Output>(output.output()));
    if(basic_output_.get_unlock_(2))return false;

    const auto expir=basic_output_.get_unlock_(3);
    if(expir)
    {
        const auto expiration_cond=*(std::dynamic_pointer_cast<qblocks::Expiration_Unlock_Condition>(expir));
        const auto unix_time=expiration_cond.unix_time();
        const auto cday=QDateTime::currentDateTime().toSecsSinceEpoch();
        if(cday+100>unix_time)return false;
    }

    return true;

}
Booking Book_Server::get_new_booking_from_metadata(qblocks::fl_array<quint16> metadata)const
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
void Book_Server::check_features(const Node_output &output)
{
    qDebug()<<"check_features";
    const auto basic_output_=std::dynamic_pointer_cast<qblocks::Basic_Output>(output.output());
    const auto metfeature=basic_output_->get_feature_(2);
    if(metfeature)
    {
        auto metadata_feature=std::dynamic_pointer_cast<qblocks::Metadata_Feature>(metfeature);
        auto metadata=metadata_feature->data();

        qDebug()<<"metadata:"<<metadata.toHex();
        auto new_book=get_new_booking_from_metadata(metadata);

        if(new_book.finish()>=QDateTime::currentDateTime()&&new_book.start().daysTo(new_book.finish())<7
                &&new_book.calculate_price(price_per_hour_)<=basic_output_->amount())
        {

            const auto stor=basic_output_->get_unlock_(1);
            if(stor)
            {
                const auto stor_cond=*(std::dynamic_pointer_cast<qblocks::Storage_Deposit_Return_Unlock_Condition>(stor));
                qDebug()<<"stor_cond.return_amount():"<<stor_cond.return_amount();
                if(basic_output_->amount()-stor_cond.return_amount()<new_book.calculate_price(price_per_hour_))
                {
                    return;
                }
            }

            auto pair=books_.insert(new_book);
            qDebug()<<"inserted:"<<pair.second;
            if(pair.second)
            {
                emit got_new_booking(new_book);
                clean_state();
                collect_output(output);
            }

        }

    }
}
std::shared_ptr<qblocks::Output> Book_Server::get_publish_output(const quint64 &amount)const
{

    const fl_array<quint8> tag("state");
    const auto state=get_state_metadata();
    const auto eddAddr=std::shared_ptr<Address>(new Ed25519_Address(account_.get_publish_addr().addr_hash));
    auto sendFea=std::shared_ptr<qblocks::Feature>(new Sender_Feature(eddAddr));
    auto tagFea=std::shared_ptr<qblocks::Feature>(new Tag_Feature(tag));
    auto metFea=std::shared_ptr<qblocks::Feature>(new Metadata_Feature(state));

    auto addUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(eddAddr));

    return std::shared_ptr<qblocks::Output>(new Basic_Output(amount,{addUnlcon},{sendFea,metFea,tagFea},{}));
}
void Book_Server::handle_new_book(QJsonValue data)
{
    auto node_out=Node_output(data);

    if(node_out.output()->type_m==3&&publishing_)
    {
        if(check_unlock_conditions(node_out))
        {
            check_features(node_out);
        }

    }



}
void Book_Server::handle_init_funds(QJsonValue data,ResponseMqtt* respo)
{

    auto info=iota_client_->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,iota_client_,[=]( ){

        auto node_outputs_=new Node_outputs();
        iota_client_->get_basic_outputs(node_outputs_,"address="+account_.get_publish_addr().addr);
        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client_,[=]( ){

            auto publish_bundle=account_.get_publish_addr();
            c_array Inputs_Commitments;
            quint64 amount=0;
            std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
            std::vector<std::shared_ptr<qblocks::Input>> inputs;
            publish_bundle.consume_outputs(node_outputs_->outs_,0,Inputs_Commitments,amount,ret_outputs,inputs);

            if(amount>=min_amount_funds_)
            {

                auto pubOut=get_publish_output(0);
                const auto min_output_pub=std::dynamic_pointer_cast<qblocks::Basic_Output>
                        (pubOut)->min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);
                auto vareddAddr=std::shared_ptr<Address>(new Ed25519_Address(account_.get_payment_addr(0).addr_hash));
                const auto varaddUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(vareddAddr));
                const auto min_deposit_pay= Basic_Output(0,{varaddUnlcon},{},{})
                        .min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);

                std::dynamic_pointer_cast<qblocks::Basic_Output>(pubOut)->set_amount(10*min_output_pub);
                auto Inputs_Commitment=c_array(QCryptographicHash::hash(Inputs_Commitments, QCryptographicHash::Blake2b_256));
                cons_step_index=(amount-10*min_output_pub)/min_deposit_pay;
                qDebug()<<"cons_step_index:"<<cons_step_index;
                std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{pubOut};
                for(auto i=0;i<cons_step_index;i++)
                {
                    auto consAddr=std::shared_ptr<Address>(new Ed25519_Address(account_.get_payment_addr(i+1).addr_hash));
                    const auto consUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(consAddr));
                    const auto consOut= std::shared_ptr<qblocks::Output>
                            (new Basic_Output((i==cons_step_index-1)?
                                                  amount-10*min_output_pub-(cons_step_index-1)*min_deposit_pay
                                                :min_deposit_pay,{consUnlcon},{},{}));
                    the_outputs_.push_back(consOut);
                }
                the_outputs_.insert( the_outputs_.end(), ret_outputs.begin(), ret_outputs.end());
                auto essence=std::shared_ptr<qblocks::Essence>(
                            new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,the_outputs_,nullptr));


                c_array serializedEssence;
                serializedEssence.from_object<Essence>(*essence);

                auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
                std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;
                publish_bundle.create_unlocks(essence_hash,unlocks);

                auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,{unlocks}));
                auto block_=Block(trpay);
                iota_client_->send_block(block_);
                set_publishing(true);
                respo->unsubscribe();

            }
            else
            {
                set_transfer_funds(min_amount_funds_-amount);
            }
            info->deleteLater();
            node_outputs_->deleteLater();
        });
    });

}
void Book_Server::collect_output(const Node_output &output)
{
    qDebug()<<"collecting output";

    auto info=iota_client_->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,iota_client_,[=]( ){

        auto node_outputs_cons= new Node_outputs();
        const auto cons_bundle=account_.get_payment_addr(consolidation_index);
        consolidation_index++;
        iota_client_->get_basic_outputs(node_outputs_cons,"address="+cons_bundle.addr);

        QObject::connect(node_outputs_cons,&Node_outputs::finished,this,[=]( ){

            auto node_outputs_publish=new Node_outputs();
            iota_client_->get_basic_outputs(node_outputs_publish,"address="+account_.get_publish_addr().addr+"&hasStorageDepositReturn=false&hasTimelock=false&hasExpiration=false&sender="
                                            +account_.get_publish_addr().addr+"&tag="+fl_array<quint8>("state").toHexString());

            QObject::connect(node_outputs_publish,&Node_outputs::finished,iota_client_,[=]( ){

                auto payment_bundle=account_.get_payment_addr(0);
                auto consolidation_bundle=cons_bundle;
                auto publish_bundle=account_.get_publish_addr();


                c_array Inputs_Commitments;
                quint64 amount=0;
                std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
                std::vector<std::shared_ptr<qblocks::Input>> inputs;

                payment_bundle.consume_outputs({output},0,Inputs_Commitments,amount,ret_outputs,inputs);
                consolidation_bundle.consume_outputs(node_outputs_cons->outs_,0,Inputs_Commitments,amount,ret_outputs,inputs);
                publish_bundle.consume_outputs(node_outputs_publish->outs_,0,Inputs_Commitments,amount,ret_outputs,inputs);

                if(amount)
                {
                    auto Inputs_Commitment=c_array(QCryptographicHash::hash(Inputs_Commitments, QCryptographicHash::Blake2b_256));

                    auto consAddr=std::shared_ptr<Address>(new Ed25519_Address(account_.get_payment_addr(consolidation_index+cons_step_index-1).addr_hash));
                    auto consaddrUnlcon=std::shared_ptr<qblocks::Unlock_Condition>(new Address_Unlock_Condition(consAddr));
                    const auto min_deposit= Basic_Output(0,{consaddrUnlcon},{},{})
                            .min_deposit_of_output(info->vByteFactorKey,info->vByteFactorData,info->vByteCost);

                    const auto consOut= std::shared_ptr<qblocks::Output>(new Basic_Output(min_deposit,{consaddrUnlcon},{},{}));
                    const auto pubOut=get_publish_output(amount-min_deposit);

                    std::vector<std::shared_ptr<qblocks::Output>> the_outputs_{consOut,pubOut};
                    the_outputs_.insert( the_outputs_.end(), ret_outputs.begin(), ret_outputs.end());

                    auto essence=std::shared_ptr<qblocks::Essence>(
                                new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,the_outputs_,nullptr));

                    c_array serializedEssence;
                    serializedEssence.from_object<Essence>(*essence);
                    auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);

                    std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;
                    payment_bundle.create_unlocks(essence_hash,unlocks);
                    consolidation_bundle.create_unlocks(essence_hash,unlocks);
                    publish_bundle.create_unlocks(essence_hash,unlocks);

                    auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,{unlocks}));
                    auto block_=Block(trpay);
                    iota_client_->send_block(block_);

                }
                node_outputs_cons->deleteLater();
                node_outputs_publish->deleteLater();
                info->deleteLater();
            });

        });


    });



}

void Book_Server::publish_state()
{
    clean_state();

    auto info=iota_client_->get_api_core_v2_info();
    QObject::connect(info,&Node_info::finished,iota_client_,[=]( ){

        auto node_outputs_=new Node_outputs();
        iota_client_->get_basic_outputs(node_outputs_,"address="+account_.get_publish_addr().addr+"&hasStorageDepositReturn=false&hasTimelock=false&hasExpiration=false&sender="
                                        +account_.get_publish_addr().addr+"&tag="+fl_array<quint8>("state").toHexString());

        QObject::connect(node_outputs_,&Node_outputs::finished,iota_client_,[=]( ){

            auto publish_bundle=account_.get_publish_addr();
            c_array Inputs_Commitments;
            quint64 amount=0;
            std::vector<std::shared_ptr<qblocks::Output>> ret_outputs;
            std::vector<std::shared_ptr<qblocks::Input>> inputs;
            publish_bundle.consume_outputs(node_outputs_->outs_,0,Inputs_Commitments,amount,ret_outputs,inputs);

            if(amount)
            {
                auto Inputs_Commitment=c_array(QCryptographicHash::hash(Inputs_Commitments, QCryptographicHash::Blake2b_256));

                auto BaOut=get_publish_output(amount);
                ret_outputs.push_back(BaOut);
                auto essence=std::shared_ptr<qblocks::Essence>(new Transaction_Essence(info->network_id_,inputs,Inputs_Commitment,
                                                                                       ret_outputs,nullptr));
                c_array serializedEssence;
                serializedEssence.from_object<Essence>(*essence);

                auto essence_hash=QCryptographicHash::hash(serializedEssence, QCryptographicHash::Blake2b_256);
                std::vector<std::shared_ptr<qblocks::Unlock>> unlocks;

                publish_bundle.create_unlocks(essence_hash,unlocks);

                auto trpay=std::shared_ptr<qblocks::Payload>(new Transaction_Payload(essence,unlocks));

                auto block_=Block(trpay);
                iota_client_->send_block(block_);


            }
            info->deleteLater();
            node_outputs_->deleteLater();
        });

    });

}
bool Book_Server::try_to_open(QString code)
{
    qDebug()<<"books_.size():"<<books_.size();
    clean_state();
    qDebug()<<"books_.size():"<<books_.size();
    auto now=QDateTime::currentDateTime();
    if(books_.size()&&(books_.begin()->start()<=now)&&(books_.begin()->finish()>=now)&&books_.begin()->verify_code_str(code))
    {
        open=true;
        return true;
        emit open_changed(open);
    }

    return false;
}
