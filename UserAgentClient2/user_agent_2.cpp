

/* 
 * File:   user_agent_2.cpp
 * Author: hamza
 *
 * Created on October 25, 2025, 6:02 PM
 */

#include <cstdlib>
#include <pjsua2.hpp>
#include<iostream>
using namespace std;

using namespace pj;

string myIp = "192.168.0.157:6000";
string from = "sip:hamza@client.com";

class ClientEndpoint : public Endpoint {
public:

    ClientEndpoint() {
    };

    ~ClientEndpoint() {
    };

    PJ_DECL(pjsip_endpoint*) getPjsipEdnpoint() {
        return pjsua_get_pjsip_endpt();
    }

};

static pj_status_t make_call(string target, string to);

void sendRequest(pjsip_method_e methodId, string methodName, string target, string to) {
    pjsip_tx_data* tdata;
    pjsip_method method;
    method.id = methodId;
    method.name = pj_str((char*) methodName.c_str());
    pj_str_t target_uri = pj_str((char*) target.c_str());
    pj_str_t from_uri = pj_str((char*) from.c_str());
    pj_str_t to_uri = pj_str((char*) to.c_str());
    string temp = "sip:" + myIp;
    pj_str_t contact_uri = pj_str((char*) temp.c_str());
    pj_status_t status = pjsip_endpt_create_request(
            pjsua_get_pjsip_endpt(),
            &method,
            &target_uri,
            &from_uri,
            &to_uri,
            &contact_uri,
            NULL, NULL, NULL,
            &tdata
            );
    if (status != PJ_SUCCESS) {
        char errmsg[PJ_ERR_MSG_SIZE];
        pj_strerror(status, errmsg, sizeof (errmsg));
        cout << "***********DAMN*********** Error: " << errmsg << endl;
    } else {
        cout << "***********HELL YEAH***********" << endl;
        status = pjsip_endpt_send_request(pjsua_get_pjsip_endpt(), tdata, -1, NULL, NULL);
        if (status != PJ_SUCCESS) {
            cout << "Failed to send request!" << endl;
        }
    }
}


pj_bool_t on_rx_request(pjsip_rx_data *rdata);
pj_bool_t on_rx_response(pjsip_rx_data *rdata);


pjsip_module client_module = {
    NULL, NULL, // prev, next (linked list)
    pj_str("uac-app-module"), // Name
    -1, // ID (will be set by registration)
    PJSIP_MOD_PRIORITY_UA_PROXY_LAYER, // Priority
    NULL, // load()
    NULL, // start()
    NULL, // stop()
    NULL, // unload()
    &on_rx_request, // on_rx_request()
    &on_rx_response, // on_rx_response()
    NULL, // on_tx_request()
    NULL, // on_tx_response()
    NULL, // on_tsx_state()
};


ClientEndpoint ep;

int main(int argc, char** argv) {

    ep.libCreate();

    EpConfig epConfig;
    epConfig.logConfig.level = 4;

    ep.libInit(epConfig);

    TransportConfig tpConfig;
    tpConfig.port = 6000;
    ep.transportCreate(PJSIP_TRANSPORT_UDP, tpConfig);
    //    pjsip_ua_init_module(ep.getPjsipEdnpoint(), NULL);

    pjsip_endpt_register_module(ep.getPjsipEdnpoint(), &client_module);

    cout << "Module registered with ID: " << client_module.id << endl;
    if (client_module.id < 0) {
        cout << "ERROR: Module ID is still -1!" << endl;
        return 1;
    }

    ep.libStart();

    if (pjsip_ua_instance() == NULL) {
        // Not initialized, do it now
        pjsip_ua_init_module(ep.getPjsipEdnpoint(), NULL);
        cout << "UA module initialized" << endl;
    } else {
        cout << "UA module already initialized" << endl;
    }


    AccountConfig accConfig;
    accConfig.idUri = "sip:" + myIp;

    Account* acc = new Account();
    acc->create(accConfig);

    char x;
    while (cin >> x) {
        if (x) {
            make_call("sip:192.168.0.157:5000", "sip:hamza@linphone.com");
        }
    }

    ep.libDestroy();

    return 0;
}

void send_ack(pjsip_dialog* dlg, pjsip_rx_data *rdata) {
    pj_status_t status;
    pjsip_tx_data *tdata;
    status = pjsip_dlg_create_request(dlg, &pjsip_ack_method, rdata->msg_info.cseq->cseq, &tdata);
    if (status != PJ_SUCCESS) {
        cout << "Failed to create ACK request" << endl;
    }
    status = pjsip_dlg_send_request(dlg, tdata, -1, NULL);
    if (status != PJ_SUCCESS) {
        cout << "Failed to send ACK request" << endl;
    }
}

pj_bool_t on_rx_request(pjsip_rx_data *rdata) {

    pj_status_t status;
    if (rdata->msg_info.msg->line.req.method.id == PJSIP_BYE_METHOD) {
        cout << "************************************************************************ RECEIVED BYE REQUEST -- ending call" << endl;
        pjsip_dialog *dlg;
        dlg = pjsip_rdata_get_dlg(rdata);
        if (dlg == NULL) {

            status = pjsip_endpt_respond_stateless(pjsua_get_pjsip_endpt(), rdata, 481, NULL, NULL,
                    NULL);
            return PJ_TRUE;
        }
        cout << "DIALOG FOUND" << endl;
        // dialog found
        pjsip_transaction *tsx;
        tsx = pjsip_rdata_get_tsx(rdata);

        if (tsx == NULL) {
            cout << "ERROR: No transaction found!" << endl;
            return PJ_FALSE;
        }

        pjsip_tsx_recv_msg(tsx, rdata);

        status = pjsip_dlg_respond(dlg, rdata, 200, NULL, NULL, NULL);
        if (status != PJ_SUCCESS) {
            cout << "FAILD TO CLOSE CALL" << endl;
        }
        pjsip_dlg_dec_session(dlg, &client_module);
        return PJ_TRUE;
    }
    return PJ_FALSE;
};

pj_bool_t on_rx_response(pjsip_rx_data *rdata) {

    pj_status_t status;

    pjsip_dialog* dlg = pjsip_rdata_get_dlg(rdata);

    if (dlg != NULL) {
        pjsip_transaction *tsx = pjsip_rdata_get_tsx(rdata);
        if (tsx != NULL) {
            if (tsx->method.id == PJSIP_INVITE_METHOD) {
                if (tsx->status_code < 200) {
                    cout << "Received provisional response " << tsx->status_code << endl;
                } else if (tsx->status_code > 300) {
                    cout << "Dialog failed with status" << endl;
                    pjsip_dlg_dec_session(dlg, &client_module);
                } else {
                    cout << "Received success code with status " << tsx->status_code << endl;
                    send_ack(dlg, rdata);
                    return PJ_TRUE;
                }
            }
        } else if (tsx == NULL && rdata->msg_info.cseq->method.id == PJSIP_INVITE_METHOD
                && rdata->msg_info.msg->line.status.code / 100 == 2) {

            send_ack(dlg, rdata);
        }
        return PJ_TRUE;
    }
    return PJ_FALSE;

};

static pj_status_t make_call(string target, string to) {
    cout << "========== Entering make_call ==========" << endl;

    pj_status_t status;

    pjsip_dialog *dlg;
    pjsip_tx_data *tdata;

    const pj_str_t target_uri = pj_str((char*) target.c_str());
    const pj_str_t from_uri = pj_str((char*) from.c_str());
    const pj_str_t to_uri = pj_str((char*) to.c_str());
    string temp = "sip:" + myIp;
    const pj_str_t contact_uri = pj_str((char*) temp.c_str());

    status = pjsip_dlg_create_uac(pjsip_ua_instance(), &from_uri, &contact_uri,
            &to_uri, &target_uri, &dlg);


    if (status != PJ_SUCCESS) {
        cout << "FAILED TO CREATE UAC DIALOG" << endl;
        return status;
    } else {
        cout << "CREATE UAC DIALOG DONE..." << endl;
    }

    pjsip_dlg_inc_lock(dlg);


    status = pjsip_dlg_add_usage(dlg, &client_module, NULL);

    if (status != PJ_SUCCESS) {
        cout << "FAILED TO ADD UAC DIALOG" << endl;
        return status;
    } else {
        cout << "ADD UAC DIALOG DONE..." << endl;
    }

    status = pjsip_dlg_inc_session(dlg, &client_module);

    if (status != PJ_SUCCESS) {
        cout << "FAILED TO INCREMEANT DIALOG" << endl;
        pjsip_dlg_dec_lock(dlg);
        return status;
    } else {
        cout << "INCREMEANT DIALOG DONE..." << endl;
        cout << "DLG_SESSION_COUNT: " << dlg->sess_count << endl;
    }

    status = pjsip_dlg_create_request(
            dlg,
            &pjsip_invite_method,
            -1,
            &tdata
            );

    const char* sdp_text =
            "v=0\r\n"
            "o=user 123456 123456 IN IP4 192.168.0.157\r\n"
            "s=Call\r\n"
            "c=IN IP4 192.168.1.59\r\n"
            "t=0 0\r\n"
            "m=audio 4000 RTP/AVP 0 8\r\n"
            "a=rtpmap:0 PCMU/8000\r\n"
            "a=rtpmap:8 PCMA/8000\r\n";

    pj_str_t sdp_body = pj_str((char*) sdp_text);
    const pj_str_t type = pj_str("application");
    const pj_str_t subtype = pj_str("sdp");

    tdata->msg->body = pjsip_msg_body_create(tdata->pool, &type, &subtype, &sdp_body);
    status = pjsip_dlg_send_request(dlg, tdata, -1, NULL);
    pjsip_dlg_dec_lock(dlg);
    if (status != PJ_SUCCESS) {
        cout << "FAILED TO SEND REQUEST" << endl;
        return status;
    } else {
        cout << "SEND REQUEST DONE..." << endl;
        cout << "DLG_SESSION_COUNT: " << dlg->sess_count << endl;
    }

    return status;
}