
#ifndef HBBREGISTER_CLIENT_H
#define HBBREGISTER_CLIENT_H
#include "HBBservice_client.h"

////////////////////////////////////////////////
//// ע�����Ŀͻ��� ��windows����ֱ�������ļ�
class HBBRegisterClient:public HBBServiceClient
{
public:
    static HBBRegisterClient *Get()
    {
        static HBBRegisterClient *xc = 0;
        if (!xc)
        {
            xc = new HBBRegisterClient();
        }
        return xc;
    }

    ~HBBRegisterClient();

    //���ӳɹ�����Ϣ�ص�����ҵ��������
    virtual void ConnectedCB();


    //���շ����ע����Ӧ
    void RegisterRes(HBBmsg::HBBMsgHead *head, Msg *msg);

    //��ȡ�����б����Ӧ
    void GetServiceRes(HBBmsg::HBBMsgHead *head, Msg *msg);

    static void RegMsgCallback()
    {
        RegCB(HBBmsg::MSG_REGISTER_RES, (MsgCBFunc)&HBBRegisterClient::RegisterRes);
        RegCB(HBBmsg::MSG_GET_SERVICE_RES, (MsgCBFunc)&HBBRegisterClient::GetServiceRes);
    }

    ///////////////////////////////////////////////////////////////////////
    //// ��ע������ע����� �˺�������Ҫ��һ�����ã���������
    /// @para service_name ΢��������
    /// @para port ΢����ӿ�
    /// @para ip ΢�����ip���������NULL������ÿͻ������ӵ�ַ
    void RegisterServer(const char *service_name, int port, const char *ip);

    /// ��ȡ���еķ����б�����ԭ���ݣ�ÿ�������ϴεĸ�������
    /// �˺����Ͳ���HBBServiceMap���ݵĺ�����һ���߳�
    HBBmsg::HBBServiceMap *GetAllService();


    /////////////////////////////////////////////////////////////////////////////
    /// ��ȡָ���������Ƶ�΢�����б� ������������
    /// 1 �ȴ����ӳɹ� 2 ���ͻ�ȡ΢�������Ϣ 3 �ȴ�΢�����б���Ϣ�������п����õ���һ�ε����ã�
    /// @para service_name ��������
    /// @para timeout_sec ��ʱʱ��
    /// @return �����б�
    HBBmsg::HBBServiceMap::HBBServiceList GetServcies(const char *service_name, int timeout_sec);

    /////////////////////////////////////////////////////////////
    ///�����л�ȡ΢�����б������
    ///@para service_name == NULL ��ȡȫ��
    void GetServiceReq(const char *service_name);

	//��ʱ�������ڷ�������
	virtual void TimerCB() override;
private:
    HBBRegisterClient();

    char service_name_[32] = {0};
    int service_port_ = 0;
    char service_ip_[16] = {0};

	// ��ȡ���ػ���,�̲߳���ȫ��
	bool LoadLocalFile();
	std::string getLocalFileName();

};

#endif