#include"request_handler.h"
#include <iostream>
#include <string>

using namespace std;

pjsip_uri* create_uri_from_string(pj_pool_t* pool, const std::string& uri_str) {
    // Validate inputs
    if (!pool || uri_str.empty()) {
        cout << "!!!!!!!!!!!!!!! FAILD TO CREATE TARGET URI" << endl;
        return NULL;
    }

    pjsip_uri* uri = pjsip_parse_uri(pool,
            const_cast<char*> (uri_str.c_str()),
            uri_str.length(),
            0);

    return uri;
}

void RequestHandler::register_req(pjsip_rx_data *rdata, pjsip_transaction *uas_tsx, pjsip_module *proxy_module) {
    cout << "RECEIVED REGISTER REQUEST" << endl;
    char registrar_uri[] = "sip:192.168.1.59:5060";
    pj_str_t registrar_str = pj_str(registrar_uri);
    pjsip_uri* registrar_uri_obj = pjsip_parse_uri(rdata->tp_info.pool, registrar_str.ptr, registrar_str.slen, 0);

    pjsip_tx_data* tdata;
    //    pj_str_t branch_param = {NULL, 0};  
    pj_status_t status = pjsip_endpt_create_request_fwd(
            pjsua_get_pjsip_endpt(),
            rdata,
            registrar_uri_obj,
            NULL,
            0,
            &tdata
            );

    pjsip_transaction *uac_tsx;
    pjsip_tsx_create_uac(proxy_module, tdata, &uac_tsx);
    uac_tsx->mod_data[proxy_module->id] = (void*) uas_tsx;
    uas_tsx->mod_data[proxy_module->id] = (void*) uac_tsx;

    cout << "Module ID: " << proxy_module->id << endl;
    cout << "UAC tsx: " << uac_tsx << endl;
    cout << "UAS tsx: " << uas_tsx << endl;
    cout << "Linked UAS in UAC mod_data: " << uac_tsx->mod_data[proxy_module->id] << endl;


    if (status != PJ_SUCCESS) {
        cout << "!!!!!!!!!!!!!! Failed to create forwarded request" << endl;
        return;
    } else {
        cout << "***************** Successfully created forwarded request" << endl;

    }

    status = pjsip_tsx_send_msg(uac_tsx, tdata);
    if (status != PJ_SUCCESS) {
        cout << "!!!!!!!!!!!!!!!!! Failed to send forwarded request" << endl;
        return;
    }

    cout << "*********************  REGISTER forwarded successfully" << endl;
}

void RequestHandler::invite_req(pjsip_rx_data *rdata, pjsip_transaction *uas_tsx, pjsip_module *proxy_module, string target) {
    cout << "RECEIVED INVITE REQUEST" << endl;
    pjsip_uri *target_uri = create_uri_from_string(rdata->tp_info.pool, target);
    pjsip_tx_data *tdata;
    pj_status_t status = pjsip_endpt_create_request_fwd(pjsua_get_pjsip_endpt(), rdata, target_uri, NULL, 0, &tdata);
    if (status != PJ_SUCCESS) {
        cout << "FAILED TO CREATE INVITE REQUEST" << endl;
    } else {
        cout << "CREATING INVITE REQUEST DONE..." << endl;
    }
    pjsip_transaction* uac_tsx;
    status = pjsip_tsx_create_uac(proxy_module, tdata, &uac_tsx);
    if (status != PJ_SUCCESS) {
        cout << "FAILED TO CREATE UAC TRANSACTION" << endl;
    } else {
        cout << "UAC DONE..." << endl;
    }
    uac_tsx->mod_data[proxy_module->id] = (void*) uas_tsx;
    uas_tsx->mod_data[proxy_module->id] = (void*) uac_tsx;
    status = pjsip_tsx_send_msg(uac_tsx, tdata);
    if (status != PJ_SUCCESS) {
        cout << "FAILED TO FORWARED INVITE REQUEST" << endl;
    } else {
        cout << "INVITE REQUEST FORWARDING DONE..." << endl;
    }
}

void RequestHandler::other_req(pjsip_rx_data *rdata) {
    cout << "RECEIVED OTHER REQUEST" << endl;
}

void RequestHandler::options_req(pjsip_rx_data *rdata) {
    cout << "RECEIVED OPTIONS REQUEST" << endl;
}

void RequestHandler::bye_req(pjsip_rx_data *rdata) {
    cout << "RECEIVED BYE REQUEST" << endl;
}

void RequestHandler::cancel_req(pjsip_rx_data *rdata) {
    cout << "RECEIVED CANCEL REQUEST" << endl;
}

void RequestHandler::ack_req(pjsip_rx_data *rdata) {
    cout << "RECEIVED ACK REQUEST" << endl;
}