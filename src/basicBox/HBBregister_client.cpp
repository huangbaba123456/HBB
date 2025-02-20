
#include "HBBregister_client.h"
#include "HBBmsg.pb.h"
#include "HBBtools.h"
#include <thread>
#include <fstream>
#include <sstream>
#include "HBBlog_client.h"

using namespace HBBmsg;

using namespace std;

///ע������б�Ļ���
static HBBServiceMap * service_map = 0;
static HBBServiceMap * client_map = 0;

//���̷߳��ʵ���
static mutex service_map_mutex;

/////////////////////////////////////////////////////////////
///�����л�ȡ΢�����б������
///@para service_name == NULL ��ȡȫ��
void HBBRegisterClient::GetServiceReq(const char *service_name)
{

    HBBGetServiceReq req;
    if (service_name)
    {
        req.set_type(HBBServiceType::ONE);
        req.set_name(service_name);
    }
    else
    {
        req.set_type(HBBServiceType::ALL);
    }

    SendMsg(MSG_GET_SERVICE_REQ, &req);
}
/////////////////////////////////////////////////////////////////////////////
/// ��ȡָ���������Ƶ�΢�����б� ������������
/// 1 �ȴ����ӳɹ� 2 ���ͻ�ȡ΢�������Ϣ 3 �ȴ�΢�����б���Ϣ�������п����õ���һ�ε����ã�
/// @para service_name ��������
/// @para timeout_sec ��ʱʱ��
/// @return �����б�
HBBmsg::HBBServiceMap::HBBServiceList HBBRegisterClient::GetServcies(const char *service_name, int timeout_sec)
{
    HBBmsg::HBBServiceMap::HBBServiceList re;
    //ʮ�����ж�һ��
    int totoal_count = timeout_sec * 100;
    int count = 0;
    //1 �ȴ����ӳɹ�
    while (count < totoal_count)
    {
        //cout << "@" << flush;
        if (is_connected())
            break;
        this_thread::sleep_for(chrono::milliseconds(10));
        count++;
    }
	// ��Ҫ�ж��Ƿ�service_name�Ƿ�Ϊ��
    if (!is_connected())
    {
        LOGDEBUG("���ӵȴ���ʱ");
		{
			HBBMutex mutex(&service_map_mutex);
			if (!service_map) {
				service_map = new HBBServiceMap();
			}
			LoadLocalFile();
			if (service_map) {
				auto m = service_map->mutable_service_map();
				auto s = m->find(service_name);
				if (s != m->end()) {
					re.CopyFrom(s->second);
				}
			}
		}
        return re;
    }

    //2 ���ͻ�ȡ΢�������Ϣ
    GetServiceReq(service_name);
    
    //3 �ȴ�΢�����б���Ϣ�������п����õ���һ�ε����ã�
    while (count < totoal_count)
    {
        cout << "." << flush;
        HBBMutex mutex(&service_map_mutex);
        if (!service_map)
        {
            this_thread::sleep_for(chrono::milliseconds(10));
            count++;
            continue;
        }
        auto m = service_map->mutable_service_map();
        if (!m)
        {
            //cout << "#" << flush;
            //û���ҵ�ָ����΢����
            GetServiceReq(service_name);
            this_thread::sleep_for(chrono::milliseconds(100));
            count+=10;
            continue;
        }
        auto s = m->find(service_name);
        if (s == m->end())
        {
           // cout << "+" << flush;
            //û���ҵ�ָ����΢����
            GetServiceReq(service_name);
            this_thread::sleep_for(chrono::milliseconds(100));
            count += 10;
            continue;
        }
        re.CopyFrom(s->second);
        return re;
    }
    return re;

    //XMutex mutex(&service_map_mutex);
}

HBBmsg::HBBServiceMap *HBBRegisterClient::GetAllService()
{
    HBBMutex mutex(&service_map_mutex);
	if (!service_map) {
		LoadLocalFile();
	}
    if (!service_map)
    {
        return NULL;
    }
    if (!client_map)
    {
        client_map = new HBBServiceMap();
    }
    client_map->CopyFrom(*service_map);
    return client_map;
}
//��ȡ�����б����Ӧ
void HBBRegisterClient::GetServiceRes(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    LOGDEBUG("GetServiceRes");

	HBBServiceMap tmp_map;
    if (!tmp_map.ParseFromArray(msg->data, msg->size)){
        LOGDEBUG("service_map.ParseFromArray failed!");
        return;
    }
	if (tmp_map.mutable_res()->return_()==HBBMessageRes::HBBReturn::HBBMessageRes_HBBReturn_ERROR) {
		//��Ϣ���������
		LOGDEBUG("HBBMessageRes::HBBReturn::HBBMessageRes_HBBReturn_ERROR "+tmp_map.mutable_res()->msg());
		return;
	}
    //LOGDEBUG(service_map->DebugString());
	// Ҫ�������ĸ�
	{
		HBBMutex mutex(&service_map_mutex);
		if (!service_map) {
			service_map = new HBBServiceMap();
		}
	}
	
	if (tmp_map.type()== HBBServiceType::ONE) {
		// һ����
		if (tmp_map.mutable_service_map()->size() != 0) {
			auto m = tmp_map.mutable_service_map()->begin(); // ȡ��һ��
			{
				HBBMutex mutex(&service_map_mutex);
				(*(service_map->mutable_service_map()))[m->first] = m->second; // �����ȥ
			}
		}
		
	}else {
		// all��
		{
			HBBMutex mutex(&service_map_mutex);
			service_map->CopyFrom(tmp_map);
		}
	}
	// �����Ǵ��̻���
	string fileName = getLocalFileName();
	LOGDEBUG("Save local file!");
	ofstream ofs;
	ofs.open(fileName,ios::binary);
	if (!ofs.is_open()) {
		// ��ʧ��
		LOGDEBUG("save local file failed!");
	}else {
		service_map->SerializePartialToOstream(&ofs);
	}
	ofs.close();

}
//���շ����ע����Ӧ
void HBBRegisterClient::RegisterRes(HBBmsg::HBBMsgHead *head, Msg *msg)
{
    LOGDEBUG("RegisterRes");
    HBBMessageRes res;
    if (!res.ParseFromArray(msg->data, msg->size))
    {
        LOGDEBUG("HBBRegisterClient::RegisterRes failed!res.ParseFromArray failed!");
        return;
    }
    if (res.return_() == HBBMessageRes::OK)
    {
        LOGDEBUG("RegisterRes success");
        return;
    }
    stringstream ss;
    ss << "RegisterRes failed! " << res.msg();
    LOGDEBUG(ss.str().c_str());
}

void HBBRegisterClient::ConnectedCB()
{
    //����ע����Ϣ
    LOGDEBUG("connected start send MSG_REGISTER_REQ ");
    HBBRegisterReq req;
    req.set_name(service_name_);
    req.set_ip(service_ip_);
    req.set_port(service_port_);
    SendMsg(MSG_REGISTER_REQ, &req);
}

///////////////////////////////////////////////////////////////////////
//// ��ע������ע����� 
/// @para service_name ΢��������
/// @para port ΢����ӿ�
/// @para ip ΢�����ip���������NULL������ÿͻ������ӵ�ַ
void HBBRegisterClient::RegisterServer(const char *service_name, int port, const char *ip)
{
    //ע����Ϣ�ص�����
    RegMsgCallback();
    //������Ϣ��������
    //�����������Ƿ�ɹ���
    ///ע�����ĵ�IP��ע�����ĵĶ˿�
    //_CRT_SECURE_NO_WARNINGS
    if (service_name)
        strcpy(service_name_, service_name);
    if (ip)
        strcpy(service_ip_, ip);
    service_port_ = port;
    //�����Զ�����
    set_auto_connect(true);
	//  ���ö�ʱ�������������ʵ���Ƿ���������
	set_timer_ms(3000);
	// ���ö���ʱ
	// set_read_timeout_ms(5000);
    // ��������뵽�̳߳���
    StartConnect();

}

HBBRegisterClient::HBBRegisterClient()
{
}


HBBRegisterClient::~HBBRegisterClient()
{
}

bool HBBRegisterClient::LoadLocalFile() {
	bool ans;
	LOGDEBUG("Load local register data");
	if (!service_map) {
		service_map = new HBBServiceMap();
	}
	string name = getLocalFileName();
	ifstream ifs;
	ifs.open(name, ios::binary);
	if (ifs.is_open()) {
		ans = service_map->ParseFromIstream(&ifs);
		LOGDEBUG("Load local register data success");
	}else {
		ans = false;
	}
	if (!ans) {
		stringstream log;
		log << "Load local register data failed! fileName: ";
		log << name;
		LOGDEBUG(log.str());
		delete service_map;
		service_map = 0;
	}
	ifs.close();
	return ans;
}
string HBBRegisterClient::getLocalFileName() {
	stringstream ss;
	ss << "register_"<<service_name_<< "_" << service_ip_ << "_" << service_port_ << ".cache";
	return ss.str();
}

//��ʱ�������ڷ�������
void HBBRegisterClient::TimerCB()
{
	static long long count = 0;
	count++;
	HBBMsgHeart req;
	req.set_count(count);
	SendMsg(MSG_HEART_REQ, &req);
}

