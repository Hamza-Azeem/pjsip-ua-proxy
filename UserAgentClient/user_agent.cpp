#include <pjsua2.hpp>
#include <openssl/md5.h>
#include <iostream>

#include <sstream>


#include <iomanip>

using namespace pj;
using namespace std;


string myIp = "sip:192.168.1.59:5070";
string from = "sip:user-agent@client.com";

class MyAccount : public Account {
public:

    virtual void onRegState(OnRegStateParam &prm) {
        AccountInfo ai = getInfo();
        std::cout << (ai.regIsActive ? "*** Register:" : "*** Unregister:")
                << " code=" << prm.code << std::endl;
    }
};

pj_status_t on_rx_response(pjsip_rx_data *rdata);

static pjsip_module receive_mod = {
    NULL, NULL,
    pj_str("receive_mod"),
    -1,
    PJSIP_MOD_PRIORITY_UA_PROXY_LAYER,
    NULL, NULL, NULL, NULL,
    NULL,
    &on_rx_response,
    NULL, NULL, NULL
};

void sendRequest(pjsip_method_e methodId, string methodName, string target, string to) {
    pjsip_tx_data* tdata;
    pjsip_method method;
    method.id = methodId;
    method.name = pj_str((char*) methodName.c_str());
    pj_str_t target_uri = pj_str((char*) target.c_str());
    pj_str_t from_uri = pj_str((char*) from.c_str());
    pj_str_t to_uri = pj_str((char*) to.c_str());
    pj_str_t contact_uri = pj_str((char*) myIp.c_str());
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

string addSip(string x) {
    if (x.substr(0, 3) == "sip" || x.substr(0, 3) == "SIP") {
        return x;
    } else {
        x = "sip:" + x;
    }
    return x;
}

int main() {
    Endpoint ep;

    ep.libCreate();

    // Initialize endpoint
    EpConfig ep_cfg;
    ep_cfg.logConfig.level = 4;
    ep.libInit(ep_cfg);

    // Create SIP transport. Error handling sample is shown
    TransportConfig tcfg;
    tcfg.port = 5070;
    try {
        ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    } catch (Error &err) {
        std::cout << err.info() << std::endl;
        return 1;
    }

        pjsip_endpt_register_module(pjsua_get_pjsip_endpt(), &receive_mod);

    // Start the library (worker threads etc)
    ep.libStart();
    std::cout << "*** PJSUA2 STARTED ***" << std::endl;


    // Configure an AccountConfig
    AccountConfig acfg;
    acfg.idUri = "sip:hamza@edafa.com";
    acfg.regConfig.registrarUri = "sip:192.168.1.59:5000";
    //    acfg.regConfig.registrarUri = "sip:192.168.0.157";
    AuthCredInfo cred("digest", "*", "hamza", 0, "password");
    acfg.sipConfig.authCreds.push_back(cred);

    // Create the account
    MyAccount *acc = new MyAccount;
    acc->create(acfg);

    // Here we don't have anything else to do..

    cout << "\n*** PJSIP is running. Listening on UDP port 5070. ***" << endl;

    cout << "*** Configured to register to: sip:192.168.1.59:5060 ***" << endl;

    cout << "*** Press 'q' and ENTER to quit. ***\n" << endl;

    string input;

    while (true) {
        cout << "Enter SIP method (INVITE/BYE/OPTIONS/ACK/REGISTER/CANCEL/q to quit): ";
        cin >> input;

        if (input == "q") break;

        sendRequest(PJSIP_REGISTER_METHOD, "REGISTER", "sip:192.168.1.59:5000", "sip:hamza@edafa.com");

    }



    // Delete the account. This will unregister from server
    delete acc;

    // This will implicitly shutdown the library
    return 0;
}

pj_status_t on_rx_response(pjsip_rx_data *rdata) {
    string repsonseTo(rdata->msg_info.cseq->name.ptr, rdata->msg_info.cseq->name.slen);
    cout << "****** GOT RESPONSE = " << repsonseTo << endl;
}