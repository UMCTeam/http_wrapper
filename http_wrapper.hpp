#ifndef CURL_WARPPER_HPP
#define CURL_WARPPER_HPP

#include <curl/curl.h>
#include <string>
#include <map>
#include <thread>
#include <functional>
#include <future>
#include <sstream>
#include <tuple>
#include <csignal>

using namespace std;

namespace HttpWrapper {
#define DEFAULT_HANDLER() \
([](const Error&, const Response&){})
    static once_flag once;
    typedef enum {
        GET,
        POST
    } Methods;

    typedef struct {} Body;
    typedef map<string, string> Form;
    typedef map<string, string> Params;
    typedef map<string, string> Headers;

    typedef struct {
        const char* url;
        Params  params;
        Headers headers;
        Body body;
        Form form;

    } Options;

    typedef struct _Error{
        _Error(): code(0), message("") {};
        operator bool() const {
            return this->code != 0;
        }
        string message;
        int code = 0;
    } Error;

    typedef struct _Response{
        _Response(): data("") {};
        string data;
    } Response;

    typedef function<void(const Error&, const Response&)> Handler;

    void signalExit(int signum) {
        curl_global_cleanup();
        exit(signum);
    }

    size_t write_data(char* data, size_t size, size_t nmemb, string* usr) {
        if (usr == NULL) return 0;

        size_t len = size * nmemb;
        usr->append(data, len);

        return len;
    }

    future<tuple<Error, Response>> httpGet(const Options& opts, const Handler& handler) {
        packaged_task<tuple<Error, Response>(const Options&, const Handler&)> task(
                [](const Options& opts, const Handler& handler)
            -> tuple<Error, Response> {
            CURL* curl;
            CURLcode res;
            string buffer;
            Response response;
            Error error;

            curl = curl_easy_init();
            if (curl) {
                struct curl_slist* chunk = NULL;
                {
                    for(auto header : opts.headers) {
                        stringstream ss;
                        ss << header.first << ":" << header.second;
                        chunk = curl_slist_append(chunk, ss.str().c_str());
                    }
                }

                string url(opts.url);
                {
                    if (opts.params.size() > 0) {
                        url.append("?");
                        for(auto p : opts.params) {
                            url.append(p.first + "=" + p.second + "&");
                        }
                    }
                }

                curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
                curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

                res = curl_easy_perform(curl);
                if (res != CURLE_OK) {
                    error.code = res;
                } else {
                    response.data = buffer;
                }

                curl_easy_cleanup(curl);
            }
            handler(error, response);
            return tuple<Error, Response>(error, response);
        });

        future<tuple<Error, Response>> ret = task.get_future();
        thread* t = new thread(move(task), ref(opts), ref(handler));
        t->detach();

        return ret;
    }

    future<tuple<Error, Response>> httpPost(const Options& opts, const Handler& handler) {
        packaged_task<tuple<Error, Response>(const Options&, const Handler&)> task(
                [](const Options& opts, const Handler& handler)
                        -> tuple<Error, Response> {
                    CURL* curl;
                    CURLcode res;
                    string buffer;
                    Response response;
                    Error error;

                    curl = curl_easy_init();
                    if (curl) {
                        struct curl_slist* chunk = NULL;
                        {
                            for(auto header : opts.headers) {
                                stringstream ss;
                                ss << header.first << ":" << header.second;
                                chunk = curl_slist_append(chunk, ss.str().c_str());
                            }
                        }

                        const string contentType = opts.headers.at("content-type");
                        if (contentType.compare("multipart/form-data")) {
                            if (opts.form.size()) {
                                curl_mime* form =  NULL;
                                curl_mimepart* field = NULL;

                                form = curl_mime_init(curl);
                                for (auto f : opts.form) {
                                    field = curl_mime_addpart(form);
                                    curl_mime_name(field, f.first.c_str());
                                    curl_mime_data(field, f.second.c_str(), CURL_ZERO_TERMINATED);
                                }
                                curl_easy_setopt(curl, CURLOPT_MIMEPOST, form);
                            }
                        } else {
                            string forms;
                            {
                                if (opts.form.size() ) {
                                    for (auto f : opts.form) {
                                        forms.append(f.first + "=" + f.second + "&");
                                    }
                                    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, forms.c_str());
                                }
                            }
                        }

                        curl_easy_setopt(curl, CURLOPT_URL, opts.url);
                        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
                        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
                        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer);

                        res = curl_easy_perform(curl);
                        if (res != CURLE_OK) {
                            error.code = res;
                        } else {
                            response.data = buffer;
                        }
                        curl_easy_cleanup(curl);
                    }

                    handler(error, response);
                    return tuple<Error, Response>(error, response);
                });

        future<tuple<Error, Response>> ret = task.get_future();
        thread* t = new thread(move(task), ref(opts), ref(handler));
        t->detach();

        return ret;
    }

    template <Methods methods>
    future<tuple<Error, Response>> request(const Options& opts, const Handler& handler = DEFAULT_HANDLER()) {
        call_once(once, [&](){
            curl_global_init(CURL_GLOBAL_DEFAULT);
            signal(SIGTERM, signalExit);
        });
        switch(methods) {
            case Methods::GET:
            {
                return httpGet(opts, handler);
            }
            case Methods::POST:
            {
                return httpPost(opts, handler);
            }
        }
    }

};

#endif //CURL_WARPPER_HPP