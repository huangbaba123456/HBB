#include "HBBlog_handle.h"
#include "HBBmsg.pb.h"
#include "HBBlog_dao.h"
#include <iostream>
using namespace std;
using namespace HBBmsg;

void HBBLogHandle::AddLogReq(HBBmsg::HBBMsgHead* head, Msg* msg) {
	HBBAddLogReq req;
	if (!req.ParseFromArray(msg->data, msg->size)) {
		cerr << "AddLogReq failed! ParseFromArray" << endl;
		return;
	}
	if (req.service_ip().empty()) {
		req.set_service_ip(client_ip());
	}
	HBBLogDao::Get()->AddLog(&req);
}