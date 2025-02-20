#ifndef HBBSERVICEPROXY_H
#define HBBSERVICEPROXY_H
#include <map>
#include <vector>
#include <string>
#include "HBBservice_proxy_client.h"
#include "HBBrouter_handle.h"
class HBBServiceProxy
{
public:
    static HBBServiceProxy *Get()
    {
        static HBBServiceProxy xs;
        return &xs;
    }
    HBBServiceProxy();
    ~HBBServiceProxy();
    ///��ʼ��΢�����б�ע�����Ļ�ȡ������������
    bool Init();

    //���ؾ����ҵ��ͻ������ӣ��������ݷ���
    bool SendMsg(HBBmsg::HBBMsgHead *head, Msg *msg, HBBRouterHandle *ev);



    //�����Զ��������߳�
    void Start();

    //ֹͣ�߳�
    void Stop();

    void Main();

private:

    bool is_exit_ = false;
	std::map < std::string, std::vector<HBBmsg::HBBServiceMap::HBBService>> service_map_;
	std::mutex service_map_mutex_;
	//��¼�ϴ���ѯ������
	std::map<std::string, int>service_map_last_index_;

};

#endif