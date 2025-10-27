/*
 * Click nbfs://nbhost/SystemFileSystem/Templates/Licenses/license-default.txt to change this license
 * Click nbfs://nbhost/SystemFileSystem/Templates/cppFiles/file.h to edit this template
 */

/* 
 * File:   request_handler.h
 * Author: hamza
 *
 * Created on October 21, 2025, 2:28 PM
 */

#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H
#include <pjsua2.hpp>
#include <string>
using namespace std;

class RequestHandler {
public:

    RequestHandler() {
    };

    ~RequestHandler() {
    };

    void register_req(pjsip_rx_data *rdata, pjsip_transaction *uas_tsx, pjsip_module *proxy_module);
    void invite_req(pjsip_rx_data *rdata, pjsip_transaction *uas_tsx, pjsip_module *proxy_module, string target);
    void cancel_req(pjsip_rx_data *rdata);
    void other_req(pjsip_rx_data *rdata);
    void ack_req(pjsip_rx_data *rdata);
    void bye_req(pjsip_rx_data *rdata);
    void options_req(pjsip_rx_data *rdata);

};

#endif /* REQUEST_HANDLER_H */

