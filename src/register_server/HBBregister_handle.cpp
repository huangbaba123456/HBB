
#include "HBBregister_handle.h"
#include "HBBtools.h"
#include "HBBmsg.pb.h"
#include "HBBlog_client.h"

using namespace HBBmsg;
using namespace std;

///ע������б�Ļ���
static HBBServiceMap * service_map = 0;

//���̷߳��ʵ���
static mutex service_map_mutex;


//���շ���ķ�������
void HBBRegisterHandle::GetServiceReq(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    //��ʱֻ����ȫ��
    LOGDEBUG("���շ���ķ�������");
    HBBGetServiceReq req;

    //������
    HBBmsg::HBBServiceMap res;
    res.mutable_res()->set_return_(HBBMessageRes_HBBReturn_ERROR);
	res.set_type(req.type()); // ǿ��ת��
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        stringstream ss;
        ss << "req.ParseFromArray failed!";
        LOGDEBUG(ss.str().c_str());
        res.mutable_res()->set_msg(ss.str().c_str());
        SendMsg(MSG_GET_SERVICE_RES, &res);
        return;
    }

    string service_name = req.name();
    stringstream ss;
    ss << "GetServiceReq : service name " << service_name;
    LOGDEBUG(ss.str().c_str());
	{
		//����ȫ��΢��������
		HBBMutex mtx(&service_map_mutex);
		if (!service_map) {
			service_map = new HBBServiceMap();
		}
		HBBServiceMap sendMap;
		sendMap.mutable_res()->set_return_(HBBMessageRes_HBBReturn_OK);
		sendMap.set_type(req.type());
		if (req.type() == HBBServiceType::ALL) {
			// ����ȫ��
			sendMap.CopyFrom(*service_map);
			sendMap.set_type(req.type()); // ֮ǰ��һ������д��������
		}else {
			// �������ķ���
			auto tmpMap=service_map->mutable_service_map();
			if (tmpMap->find(service_name) != tmpMap->end()) {
				//
				(*sendMap.mutable_service_map())[service_name] = (*tmpMap)[service_name];
			}
		}
        LOGDEBUG(sendMap.DebugString());
		SendMsg(MSG_GET_SERVICE_RES, &sendMap);
	}
}

//���շ����ע������
void HBBRegisterHandle::RegisterReq(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    LOGDEBUG("����˽��յ��û���ע������");

    //��Ӧ����Ϣ
    HBBMessageRes res;
    
    //��������
    HBBRegisterReq req;
    if (!req.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("HBBRegisterReq ParseFromArray failed!");
        res.set_return_(HBBMessageRes::ERROR);
        res.set_msg("HBBRegisterReq ParseFromArray failed!");
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }

    
    //���յ��û��ķ������ơ�����IP������˿�
    string service_name = req.name();
    if (service_name.empty())
    {
        string error = "service_name is empty!";
        LOGDEBUG(error.c_str());
        res.set_return_(HBBMessageRes::ERROR);
        res.set_msg(error);
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }
    string service_ip = req.ip();
    if (service_ip.empty())
    {
        LOGDEBUG("service_ip is empty : client ip");
        service_ip = this->client_ip();
    }

    int service_port = req.port();
    if (service_port <= 0 || service_port > 65535)
    {
        stringstream ss;
        //string error = "service_port is error!";
        ss << "service_port is error!" << service_port;
        LOGDEBUG(ss.str().c_str());
        res.set_return_(HBBMessageRes::ERROR);
        res.set_msg(ss.str());
        SendMsg(MSG_REGISTER_RES, &res);
        return;
    }
    
    //�����û�ע����Ϣ����
    stringstream ss;
    ss << "���յ��û�ע����Ϣ:" << service_name << "|" << service_ip << ":" << service_port;
    LOGINFO(ss.str().c_str());


    //�洢�û�ע����Ϣ������Ѿ�ע����Ҫ����
    {
        HBBMutex mutex(&service_map_mutex);
        if (!service_map)
            service_map = new HBBServiceMap();
        //map��ָ��
        auto smap = service_map->mutable_service_map();

        //�Ƿ���ͬ�����Ѿ�ע��
        //��Ⱥ΢����
        auto service_list = smap->find(service_name);
        if (service_list == smap->end())
        {
            //û��ע���
            (*smap)[service_name] = HBBServiceMap::HBBServiceList();
            service_list = smap->find(service_name);
        }
        auto services = service_list->second.mutable_service();
        //�����Ƿ���ͬip�Ͷ˿ڵ�
        for (auto service : (*services))
        {
            if (service.ip() == service_ip && service.port() == service_port)
            {
                stringstream ss;
                ss <<service_name<<"|"<< service_ip << ":"
                    << service_port << "΢�����Ѿ�ע���";
                LOGDEBUG(ss.str().c_str());
                res.set_return_(HBBMessageRes::ERROR);
                res.set_msg(ss.str());
                SendMsg(MSG_REGISTER_RES, &res);
                return;
            }
        }
        //����µ�΢����
        auto ser  = service_list->second.add_service();
        ser->set_ip(service_ip);
        ser->set_port(service_port);
        ser->set_name(service_name);
        stringstream ss;
        ss << service_name << "|" << service_ip << ":"
            << service_port << "�µ�΢����ע��ɹ���";
        LOGDEBUG(ss.str().c_str());
    }


    res.set_return_(HBBMessageRes::OK);
    res.set_msg("OK");
    SendMsg(MSG_REGISTER_RES, &res);
}

HBBRegisterHandle::HBBRegisterHandle()
{
}


HBBRegisterHandle::~HBBRegisterHandle()
{
}
