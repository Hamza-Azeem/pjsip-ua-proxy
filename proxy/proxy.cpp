#include <pjsua2.hpp>
#include <cstdlib>
#include<iostream>
#include "request_handler.h"
#include "response_handler.h"
using namespace std;
using namespace pj;

map<string, vector<string>> lookupDB = {
    {"<sip:hamza@edafa.com>",
        {"192.168.0.157", "hamza", "password"}},
    {"<sip:soft@linphone.com>",
        {"192.168.0.157", "lin", "pass"}},
    {"<sip:hamza@linphone.com>",
        {"192.168.0.157:5070", "lin", "pass"}}
};

string proxyIp = "192.168.0.157:5000";
RequestHandler *requestHandler = new RequestHandler();
ResponseHandler *responseHandler = new ResponseHandler();

static pj_bool_t on_rx_request(pjsip_rx_data* rdata);
static pj_bool_t on_rx_response(pjsip_rx_data* rdata);

static pjsip_module proxy_mod = {
    NULL, NULL,
    pj_str((char*) "proxy-module"),
    -1,
    PJSIP_MOD_PRIORITY_UA_PROXY_LAYER,

    NULL, NULL, NULL, NULL,

    &on_rx_request,

    &on_rx_response,

    NULL, NULL, NULL,
};

int main(int argc, char** argv) {

    Endpoint ep;
    try {
        ep.libCreate();

        EpConfig epConf;
        epConf.logConfig.level = 4;
        ep.libInit(epConf);

    } catch (Error &e) {
        cout << e.info() << endl;
    }

    TransportConfig tc;
    tc.port = 5000;
    ep.transportCreate(PJSIP_TRANSPORT_UDP, tc);

    pjsip_endpt_register_module(pjsua_get_pjsip_endpt(), &proxy_mod);

    Account proxy;
    AccountConfig proxyConfig;
    proxyConfig.idUri = "sip:proxy@192.168.0.157:5000";
    //    proxyConfig.regConfig.registrarUri = "sip:registrar@192.168.1.59";

    proxy.create(proxyConfig);

    try {
        ep.libStart();
    } catch (Error &e) {
        cout << e.info() << endl;
    }

    cout << "*************" << "PJSIP STARTED SUCCESSFULLY" << "*************" << endl;


    char input;

    while (cin >> input) {

        if (input == 'q') break;

    }

    cout << "*************" << "PJSIP CLOSED SUCCESSFULLY" << "*************" << endl;

    ep.libDestroy();

    return 0;
}

string extractHeaderString(pjsip_uri* uri, pjsip_uri_context_e headerType) {
    char buf[256];
    int len = pjsip_uri_print(headerType, uri, buf, sizeof (buf));
    if (len > 0) {
        return string(buf, len);
    }
    return "";
}

static pj_bool_t on_rx_request(pjsip_rx_data* rdata) {
    cout << "*************" << "RECEIVED REQUEST - ACTING LIKE A USER AGENT CLIENT" << "*************" << endl;
    pjsip_to_hdr *toHdr = (pjsip_to_hdr*) pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_TO, NULL);
    pjsip_endpoint* endpt = pjsua_get_pjsip_endpt();

    if (toHdr == NULL) {
        // HANDLE REQUESTS WITHOUT TO
    }
    string aor = extractHeaderString(toHdr->uri, PJSIP_URI_IN_FROMTO_HDR);
    if (lookupDB.find(aor) == lookupDB.end()) {
        // NOT FOUND
        cout << "NOT FOUND" << endl;
        pjsip_endpt_respond_stateless(endpt, rdata, 404, NULL, NULL, NULL);
        return PJ_TRUE;
    }
    // FOUND
    string toIp = lookupDB[aor][0];
    string target = "<sip:" + toIp + ">";

    pjsip_method_e method_type = rdata->msg_info.msg->line.req.method.id;
    pjsip_transaction *uas_tsx;
    pj_status_t status;
    status = pjsip_tsx_create_uas(&proxy_mod, rdata, &uas_tsx);

    if (status == PJ_SUCCESS) {
        cout << "********* Created UAS transaction successfully" << endl;
        pjsip_tsx_recv_msg(uas_tsx, rdata);

    } else {
        return PJ_TRUE;
    }
    switch (method_type) {
        case PJSIP_INVITE_METHOD:
            requestHandler->invite_req(rdata, uas_tsx, &proxy_mod, target);
            break;
        case PJSIP_CANCEL_METHOD:
            requestHandler->cancel_req(rdata);
            break;
        case PJSIP_ACK_METHOD:
            requestHandler->ack_req(rdata);
            break;
        case PJSIP_BYE_METHOD:
            requestHandler->bye_req(rdata);
            break;
        case PJSIP_REGISTER_METHOD:
            requestHandler->register_req(rdata, uas_tsx, &proxy_mod);
            break;
        case PJSIP_OPTIONS_METHOD:
            requestHandler->options_req(rdata);
            break;
        default:
            requestHandler->other_req(rdata);
            break;
    }
    return PJ_TRUE;


}

static pj_bool_t on_rx_response(pjsip_rx_data* rdata) {
    pjsip_transaction* tsx;

    tsx = pjsip_rdata_get_tsx(rdata);
    cout << "FOUDN TSX" << endl;
    if (tsx == NULL) {
        cout << "!!!!! tsx is NULL - transaction not matched yet!" << endl;
        return PJ_FALSE;
    }
    if (tsx->tsx_user == &proxy_mod) {
        pjsip_transaction *uas_tsx;
        uas_tsx = (pjsip_transaction*) tsx->mod_data[proxy_mod.id];

        cout << "UAS transaction state: " << uas_tsx->state << endl;
        cout << "UAC transaction state: " << tsx->state << endl;

        cout << "*************" << "RECEIVED RESPONSE - ACTING LIKE A USER AGENT SERVER" << "*************" << endl;
        responseHandler->forward_response(rdata, proxyIp, uas_tsx);

    } else {
        cout << " WHYYYY " << endl;
    }

    return PJ_TRUE;
}
