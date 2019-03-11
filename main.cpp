#include <iostream>
#include <string>
#include <tuple>
#include "http_wrapper.hpp"

using namespace std;

int main() {
    //Get请求
    HttpWrapper::Options get_opts;
    get_opts.url = "http://0.0.0.0:1234/get";

    //异步操作
    HttpWrapper::request<HttpWrapper::GET>(get_opts,  [=](const HttpWrapper::Error& error, const HttpWrapper::Response& response) {

        if (!error) {
            cout << response.data << endl;
        }

    });

    //同步操作
    auto ret = HttpWrapper::request<HttpWrapper::GET>(get_opts).get();
    cout << std::get<1>(ret).data << endl;

    //Post 请求
    HttpWrapper::Options post_opts;
    post_opts.url = "http://0.0.0.0:1234/post";
    post_opts.form.insert(pair<string, string>("message", "hello"));

    HttpWrapper::request<HttpWrapper::POST>(post_opts, [=](const HttpWrapper::Error& error, const HttpWrapper::Response& response) {
        cout << response.data << endl;
    });

    while (1) {
        std::this_thread::sleep_for(chrono::seconds(1));
    }

    return 0;
}