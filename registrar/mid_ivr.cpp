#include <pjsua2.hpp>

#include <pjsua-lib/pjsua.h>

#include <pjsip/sip_module.h>

#include <iostream>

#include <map>

#include <vector>

#include <pj/string.h>

#include <pjsip_simple.h>

#include <openssl/md5.h>

#include <sstream>


#include <iomanip>


using namespace pj;

using namespace std;


map<string, vector<string>> db = {
    {"<sip:hamza@edafa.com>",
        {"192.168.1.59", "hamza", "password"}},
    {"<sip:soft@linphone.com>",
        {"192.168.1.59", "lin", "pass"}}
};

class MyEndpoint : public Endpoint {
public:

    MyEndpoint() : Endpoint() {

    };

    ~MyEndpoint() {

    };

};

string md5(string &str) {

    unsigned char hash[MD5_DIGEST_LENGTH];
    MD5_CTX md5;
    MD5_Init(&md5);
    MD5_Update(&md5, str.c_str(), str.size());
    MD5_Final(hash, &md5);
    stringstream ss;
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        ss << hex << setw(2) << setfill('0') << static_cast<int> (hash[i]);
    }
    return ss.str();
}

string generateHA1(pjsip_authorization_hdr* authHdr, string aor, string realm) {
    string username = db[aor][1];
    string password = db[aor][2];
    string combined = username + ":" + realm + ":" + password;
    return md5(combined);
}

string generateHA2(pjsip_authorization_hdr* authHdr) {
    string methodType = "REGISTER";
    string digestUri(authHdr->credential.digest.uri.ptr, authHdr->credential.digest.uri. slen);
    string combined = methodType + ":" + digestUri;
    return md5(combined);

}

int authenticate(pjsip_authorization_hdr* authHdr, string aor, string realm) {
    string response(authHdr->credential.digest.response.ptr, authHdr->credential.digest.response.slen);
    string nonce(authHdr->credential.digest.nonce.ptr, authHdr->credential.digest.nonce.slen);

    string HA1 = generateHA1(authHdr, aor, realm);
    string HA2 = generateHA2(authHdr);
    string fullKey = HA1 + ":" + nonce + ":" + HA2;
    string finalHash = md5(fullKey);
    cout << "FINAL:" << finalHash << endl;
    cout << "RES:" << response << endl;
    return finalHash == response;

}

string extractHeaderString(pjsip_uri* uri, pjsip_uri_context_e headerType) {
    char buf[256];
    int len = pjsip_uri_print(headerType, uri, buf, sizeof (buf));
    if (len > 0) {
        return string(buf, len);
    }
    return "";
}

string extractRealm(string aor) {
    int index = aor.find('@');
    int length = (aor.size() - 1) - (index + 1);
    return aor.substr(index + 1, length);
};
//string extractContactUri(pjsip_rx_data* rdata) {
//    // Find the Contact header in the message
//    pjsip_contact_hdr* contact_hdr = (pjsip_contact_hdr*)
//            pjsip_msg_find_hdr(rdata->msg_info.msg, PJSIP_H_CONTACT, NULL);
//
//    if (contact_hdr && contact_hdr->uri) {
//        return extractUriString(contact_hdr->uri);
//    }
//    return "";
//}

void handleRegister(pjsip_rx_data *rdata) {

    cout << "WHAT THE HELL" << endl;

    pjsip_to_hdr *toHdr = (pjsip_to_hdr*) pjsip_msg_find_hdr(
            rdata->msg_info.msg, // The message
            PJSIP_H_TO, // Header type
            NULL // Start search from beginning
            );


    if (toHdr == NULL) {

        cout << ">>>>>>>>>>> TO HEADER IS NOT VALID <<<<<<<<<";

        return;

    }


    string aor = extractHeaderString(toHdr->uri, PJSIP_URI_IN_FROMTO_HDR);
    if (aor != "") {
        cout << "EXTRACTED URI" << endl;
        cout << "************" << aor << "*************" << endl;
    } else {
        cout << ">>>>>>>>>>> AOR is empty <<<<<<<<<";
        return;
    }


    // DO AUTHENTICATION HERE FIRST BEFORE UPDATING DATABSE
    if (db.find(aor) == db.end()) {
        cout << "NOT FOUND" << endl;
        // Send 403 Forbidden
        pjsip_endpt_respond(pjsua_get_pjsip_endpt(), // Your endpoint
                NULL,
                rdata,
                PJSIP_SC_FORBIDDEN,
                NULL,
                NULL,
                NULL,
                NULL);

        return;
    }
    cout << "FOUND IN DB" << endl;

    string realm = extractRealm(aor);

    pjsip_authorization_hdr* authHdr = (pjsip_authorization_hdr*) pjsip_msg_find_hdr(
            rdata->msg_info.msg,
            PJSIP_H_AUTHORIZATION,
            NULL
            );


    if (authHdr == NULL) {
        cout << "*******" << "NOT AUTH HEADER WAS SENT" << "*******" << endl;
        cout << "*******" << "RECONSTRUCTING RESPONSE" << "*******" << endl;
        pjsip_tx_data *tdata;


        pjsip_endpt_create_response(pjsua_get_pjsip_endpt(), rdata, PJSIP_SC_UNAUTHORIZED, NULL, &tdata);

        pjsip_www_authenticate_hdr * wwwAuthHdr = pjsip_www_authenticate_hdr_create(tdata->pool);

        pj_cstr(&wwwAuthHdr->scheme, "Digest");



        cout << "********" << "extracted realm:" << realm << "***********" << endl;

        pj_strdup2(tdata->pool, &wwwAuthHdr->challenge.digest.realm, realm.c_str());

        cout << "AHHH" << endl;

        char nonce_buf[64];
        pj_create_random_string(nonce_buf, 32);

        pj_str_t temp_nonce;
        pj_strset(&temp_nonce, nonce_buf, 32);
        pj_strdup(tdata->pool, &wwwAuthHdr->challenge.digest.nonce, &temp_nonce);


        cout << "AHHH" << endl;


        pj_cstr(&wwwAuthHdr->challenge.digest.algorithm, "MD5");
        //        pj_cstr(&wwwAuthHdr->challenge.digest.qop, "auth");
        wwwAuthHdr->challenge.digest.stale = PJ_FALSE;

        cout << "AHHH" << endl;


        pjsip_msg_add_hdr(tdata->msg, (pjsip_hdr*) wwwAuthHdr);

        cout << "AHHH" << endl;


        pj_status_t status = pjsip_endpt_send_response2(pjsua_get_pjsip_endpt(), rdata, tdata, NULL, NULL);

        if (status != PJ_SUCCESS) {
            cout << "ERROR sending response: " << status << endl;
        }


        return;

    }


    cout << "*******AUTHORIZATION HEADER RECEIVED*******" << endl;

    if (authenticate(authHdr, aor, realm) == 0) {
        cout << "*******INVALID AUTHENTICATION*******" << endl;
        pjsip_endpt_respond(pjsua_get_pjsip_endpt(), NULL, rdata, PJSIP_SC_UNAUTHORIZED, NULL, NULL, NULL, NULL);
        
        return;
    }

    cout << "*******AUTHENTICATED SUCCESSFULLY*******" << endl;

    pjsip_contact_hdr * contactHdr = (pjsip_contact_hdr*) pjsip_msg_find_hdr(
            rdata->msg_info.msg,
            PJSIP_H_CONTACT,
            NULL
            );

    string contact = extractHeaderString(contactHdr->uri, PJSIP_URI_IN_CONTACT_HDR);
    if (contact != "") {
        cout << "EXTRACTED CONTACT HEADER" << endl;
        cout << "************" << contact << "*************" << endl;
        cout << "UPDATING DATABASE WITH NEW CONTACT HEADER" << endl;
        db[aor][0] = contact;
        cout << "UPDATED CONTACT ADDRESS" << endl;
        cout << "************** PRINTING NEW DATABASE ***************" << endl;
        for (auto &p : db) {
            cout << p.first << " " << p.second[0] << " " << p.second[1] << endl;
        }
        pjsip_endpt_respond(pjsua_get_pjsip_endpt(), NULL, rdata, PJSIP_SC_OK, NULL, NULL, NULL, NULL);
    } else {
        cout << ">>>>>>>>>>> CONTACT HEADER IS NOT VALID <<<<<<<<<";
        return;
    }

    return;

}

// Callback to catch ALL incoming SIP requests

static pj_bool_t on_rx_request(pjsip_rx_data *rdata) {

    pjsip_method *method = &rdata->msg_info.msg->line.req.method;



    cout << "\n=== MODULE CALLBACK ===" << endl;

    cout << "*** Received SIP Request: ";

    cout.write(method->name.ptr, method->name.slen);

    cout << " ***" << endl;



    if (method->id == PJSIP_REGISTER_METHOD) {

        cout << ">>> This is a REGISTER request!" << endl;

        if (rdata != NULL) {

            handleRegister(rdata);

            return PJ_TRUE;

        }

    } else {

        cout << "--- Not a Register method ---" << endl;
        pjsip_endpt_respond(pjsua_get_pjsip_endpt(), NULL, rdata, PJSIP_SC_OK, NULL, NULL, NULL, NULL);


    }

    cout << "=======================" << endl;



    return PJ_FALSE; // Return PJ_FALSE to continue processing

}



static pjsip_module register_module = {

    NULL, NULL,

    pj_str((char*) "mod-app"),

    -1,

    PJSIP_MOD_PRIORITY_APPLICATION,

    NULL, NULL, NULL, NULL,

    &on_rx_request,

    NULL, NULL, NULL, NULL,

};

int main() {

    MyEndpoint endpoint;



    try {

        endpoint.libCreate();



        EpConfig ep_cfg;

        ep_cfg.logConfig.level = 4;

        endpoint.libInit(ep_cfg);



        TransportConfig tcfg;

        tcfg.port = 5060;

        endpoint.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);



        // Register module BEFORE creating account - this is important!

        pj_status_t status = pjsip_endpt_register_module(pjsua_get_pjsip_endpt(), &register_module);

        if (status != PJ_SUCCESS) {

            cout << "Failed to register module! Status: " << status << endl;

            return 1;

        }

        cout << "Module registered successfully!" << endl;



        // Create account AFTER module registration

        Account acc;

        AccountConfig acc_cfg;

        acc_cfg.idUri = "sip:registrar@192.168.1.59";

        acc.create(acc_cfg);



        endpoint.libStart();



        cout << "\n*** PJSIP is running. Listening on UDP port 5060. ***" << endl;

        cout << "*** Configure Linphone to register to: sip:192.168.1.59:5060 ***" << endl;

        cout << "*** Press 'q' and ENTER to quit. ***\n" << endl;



        char input;

        while (cin >> input) {

            if (input == 'q') break;

        }



        cout << "Shutting down..." << endl;

        endpoint.libDestroy();



    } catch (Error& e) {

        cout << "Error: " << e.info() << endl;

        return 1;

    }



    return 0;

}

