/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.cc to edit this template
 */

#include<iostream>
#include <pjsua2.hpp>
#include "response_handler.h"





//#include "string.h"

using namespace std;
using namespace pj;

bool iamSender(pjsip_via_hdr* via, string proxyIp) {
    string ip(via->recvd_param.ptr, via->recvd_param.slen);
    int port = via->rport_param;
    string viaIp = ip + ":" + to_string(port);
    return viaIp == proxyIp;
}

string extractDestUri(pjsip_via_hdr * via) {
    string ip(via->recvd_param.ptr, via->recvd_param.slen);
    int port = via->rport_param;
    string uri = ip + ":" + to_string(port);
    return uri;
}

void ResponseHandler::forward_request(pjsip_rx_data *rdata, string proxyIp) {
    //    pj_str_t branch_id = pjsip_calculate_branch_id(rdata);
    //    const pjsip_uri *uri = pjsip_parse_uri(rdata->tp_info.pool, dest, destUri.size(), 0);
    //    pj_status_t status = pjsip_endpt_create_request_fwd(pjsua_get_pjsip_endpt(), rdata, uri, &branch_id, 0, &tdata);
    //    if (status != PJ_SUCCESS) {
    //        cout << "!!!!!!!!!!!!!!!!!!! FAILED TO CREATE REQUEST" << endl;
    //        return;
    //    }
    //
    //    status = pjsip_endpt_send_request_stateless(pjsua_get_pjsip_endpt(), tdata, NULL, NULL);
    //    if (status != PJ_SUCCESS) {
    //        cout << "!!!!!!!!!!!!!!!!!!! FAILED TO SEND REQUEST" << endl;
    //        return;
    //    }
}


void ResponseHandler::forward_response(pjsip_rx_data *rdata, string proxyIp, pjsip_transaction *uas_tsx) {
    cout << "******** Received response " << endl;
    pj_status_t status;
    pjsip_tx_data* tdata;
    pjsip_endpoint* pjsip_endpt = pjsua_get_pjsip_endpt();

    status = pjsip_endpt_create_response_fwd(pjsip_endpt, rdata, 0, &tdata);
    if (status != PJ_SUCCESS) {
        cout << "!!!!!!!! FAILED TO CREATE RESPONSE" << endl;
        return;
    } else {
        cout << "*********** SUCCESSFULLY Created response" << endl;
    }

    pjsip_via_hdr* current = (pjsip_via_hdr*) pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_VIA, NULL);

    if (current != NULL) pj_list_erase(current);

    cout << "SUCCESSFULLY Striped first Via" << endl;

    status = pjsip_tsx_send_msg(uas_tsx, tdata);

    if (status != PJ_SUCCESS) {
        cout << "!!!!!!!! FAILED TO SEND RESPONSE" << endl;
        return;
    }

    cout << "******** RESPONSE FORWARDED SUCCESSFULLY" << endl;

}

void ResponseHandler::generate_quick_response(pjsip_rx_data *rdata, string proxyIp) {
}

//// Note that after this code, rdata->msg_info.via is invalid.
//pj_list_erase(rdata->msg_info.via);
//// Code above is equal to:
//// pjsip_hdr *via = pjsip_msg_find_hdr(rdata->msg, PJSIP_H_VIA);
//// pj_list_erase(via);
//// Copy the response msg.
//status = pjsip_endpt_create_response_fwd( endpt, rdata, 0, &tdata);