### 基于curl 库的http 请求封装
> 模仿nodejs中http模块发送请求的方式， 并作出优化调整

##### nodejs
```javascript
var request = require('request');
var  options = {
    method: 'post',
    url: "http://www.baidu.com",
    form: content,
    headers: {
        'Content-Type': 'application/x-www-form-urlencoded'
    }
};
          
request(options, function (err, res, body) {
    if (err) {
        console.log(err)
    }else {
        console.log(body);
    }
})
```

#### curl_wrapper
```
HttpWrapper::request<HttpWrapper::GET>(get_opts,  [=](const HttpWrapper::Error& error, const HttpWrapper::Response& response) {
        if (!error) {
            cout << response.data << endl;
        }
    });
```
##### 优化
- 预先声明请求类型
- 提供同步请求方式
- 提供显示的文件上传机制
##### 继承
- 简洁的参数 + 回调函数的方式

### 如何使用
#### 1.同步请求
```
    //Get请求
    HttpWrapper::Options get_opts;
    get_opts.url = "http://0.0.0.0:1234/get";
    //同步操作
    auto ret = HttpWrapper::request<HttpWrapper::GET>(get_opts).get();
    cout << std::get<1>(ret).data << endl;
```

#### 2.异步请求
```
    //异步操作
    HttpWrapper::request<HttpWrapper::GET>(get_opts,  [=](const HttpWrapper::Error& error, const HttpWrapper::Response& response) {
        if (!error) {
            cout << response.data << endl;
        }
    });
```

#### 4 Options 参数说明
```
struct Options {
    const char* url;  //地址
    Headers headers;  //请求头
    Params  params;   //请求参数
    Body body;        //请求体
    Form form;        //表单（dev）
}
```
