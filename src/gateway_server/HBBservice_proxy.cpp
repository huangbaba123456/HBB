#include "HBBservice_proxy.h"
#include "HBBmsg.pb.h"
#include "HBBtools.h"
#include "HBBregister_client.h"
#include <thread>
#include "HBBlog_client.h"
using namespace std;
using namespace HBBmsg;
///��ʼ��΢�����б�ע�����Ļ�ȡ������������
bool HBBServiceProxy::Init()
{

    return true;
}

//���ؾ����ҵ��ͻ������ӣ��������ݷ���
bool HBBServiceProxy::SendMsg(HBBmsg::HBBMsgHead *head, Msg *msg, HBBRouterHandle *ev)
{
    if (!head || !msg)return false;
	if (ev->serverclient()) {
		if (ev->serverclient()->is_connected()) {
			// ���Ӻ���
			LOGDEBUG("ֱ�ӷ���");
			ev->serverclient()->HBBMsgEvent::SendMsg(head,msg); // ֱ�ӷ���
			return true;
		}else if (ev->serverclient()->is_connecting()) {
			// ���������У�������Ϣ��������
			LOGDEBUG("��������Ϣ��������");
			ev->serverclient()->addMsg(head,msg);
			return true;
		}
		else {
			// �Ѿ��Ͽ�������
			//ev->set_serverclient(NULL);
			LOGDEBUG("���������Ѿ��Ͽ�");
			ev->Close();
			return false;
		}
	
	}

	string service_name = head->service_name();
	string service_ip =""; // ip��ַ
	int service_port = -1; // �˿ں�
	{
		HBBMutex mtx(&service_map_mutex_);
		auto service_list = service_map_.find(service_name);
		if (service_list == service_map_.end()) {
			LOGDEBUG("service: " + service_name + " not find");
			return false;
		}
		int cur_index = service_map_last_index_[service_name];
		// ȡ����Ӧ΢����
		HBBServiceMap::HBBService service = service_list->second[cur_index]; // ȡ�����΢����
		// �õ���Ӧ��ip �� port
		service_ip = service.ip(); // ip��ַ
		service_port = service.port(); // �˿ں�
		service_map_last_index_[service_name] = ((cur_index + 1) % service_list->second.size()); // ��������
	}
	auto proxy=HBBServiceProxyClient::Create(service_name);
	proxy->set_server_ip(service_ip.c_str());
	proxy->set_server_port(service_port);
	proxy->set_auto_delete(false); // ��Ҫ�Զ�ɾ������XRouterHandle ɾ����
	proxy->set_xrouterhandle(ev);
	ev->set_serverclient(proxy);
	proxy->StartConnect();

	// �ж��Ƿ�����
	if (ev->serverclient()) {
		if (ev->serverclient()->is_connected()) {
			// ���Ӻ���
			ev->serverclient()->HBBMsgEvent::SendMsg(head, msg); // ֱ�ӷ���
			LOGDEBUG("ֱ�ӷ���");
			return true;
		}
		else if (ev->serverclient()->is_connecting()) {
			// ���������У�������Ϣ��������
			LOGDEBUG("��������Ϣ��������");
			ev->serverclient()->addMsg(head, msg);
			return true;
		}
		else {
			// �Ѿ��Ͽ�������
			//ev->set_serverclient(NULL);
			ev->Close();
			LOGDEBUG("���������Ѿ��Ͽ�");
			return false;
		}
	}
    return true;
}


//�����Զ��������߳�
void HBBServiceProxy::Start()
{
    thread th(&HBBServiceProxy::Main, this);
    th.detach();
}

//ֹͣ�߳�
void HBBServiceProxy::Stop()
{

}

void HBBServiceProxy::Main()
{
    //�Զ�����
    while (!is_exit_)
    {
        // ��ע�����Ļ�ȡ ΢������б����
        //��������ע������
        HBBRegisterClient::Get()->GetServiceReq(0);
        auto service_map = HBBRegisterClient::Get()->GetAllService();

        if (!service_map)
        {
            LOGDEBUG("GetAllService : service_map is NULL");
            this_thread::sleep_for(1s);
            continue;
        }
        auto smap = service_map->service_map();

        if (smap.empty())
        {
            LOGDEBUG("HBBServiceProxy : service_map->service_map is NULL");
            this_thread::sleep_for(1s);
            continue;
        }
		cout << service_map->DebugString() << endl;
		{
			HBBMutex mux(&service_map_mutex_);
			service_map_.clear();
			for (auto m : smap) {
				service_map_[m.first] = vector<HBBServiceMap::HBBService>();
				//
				for (auto s : m.second.service()) {
					service_map_[m.first].push_back(s);
				}
				service_map_last_index_[m.first] = 0;
			}
			
		}
		this_thread::sleep_for(3000ms);
    }
}
HBBServiceProxy::HBBServiceProxy()
{

}


HBBServiceProxy::~HBBServiceProxy()
{

}
