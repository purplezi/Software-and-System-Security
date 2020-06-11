# Software-and-System-Security-
maybe more than 200 tasks ...

# Tutorial

<details>
<summary>2020-3-11 软件安全概述+xss攻击入门</summary>

# 软件安全

## 三大问题

### 有什么安全问题，安全问题产生的原因

#### 二进制方面

1. 内存相关问题
   - 有明确的机制
   - 例子：**缓冲区溢出**，空指针，格式化字符串
2. 逻辑错误问题
   - 多种多样

#### Web方面

1. 典型安全问题
   - **XSS**，**SQL**注入
2. 非典型安全问题：很多
   - 主要是一些代码的逻辑错误，每个漏洞都可能有不同的原理

### 如何去发现~~问题~~ (安全漏洞) -> 漏洞挖掘技术

#### 白盒分析

- 分析**软件的源代码**去寻找问题
- 方法
   - 手工代码分析 -> 软件测试技术 / 代码review。但是软件的源代码通常比较庞大，手工分析起来很费劲
   - 自动化代码分析 -> 典型的技术：符号执行

#### 黑盒分析

- 因为白盒分析数据量大，需投入很多人工，人们寻求简单方法，和白盒分析对立的黑盒发展
- 原理：完全不管软件的内部机理 / 不看源码，把需要分析的软件当做一个黑盒子，看不见 / 根本不去看内部
- 方法：分析**表面现象**
  - 软件：通过输入和输出去猜测内部机制
  - 黑盒分析法，Fuzzing技术(模糊测试)
  
### 如果有人利用这这些安全问题，如何防御

#### 补丁

- 由于软件复杂性，要发现问题及时修补
- 方法
  - 通过**漏洞数据库**来披露和管理各种漏洞，厂家有义务定期发布软件补丁或者更新
  - 软件用户应该**及时升级**软件
  - 第三方人员，如果发现了安全问题，应该**通报**给厂家，而不是在漏洞修补以前，利用漏洞搞破坏，或者不负责任的披露漏洞

#### 防御机制

1. 寻找记录攻击的痕迹，然后分析这些数据
   - 数据的来源分为了主机层面和网络层面
2. 攻击者可能会做哪些方面的伪装，常用的伪装技术有哪些，如何去对抗伪装
   - 方法：加壳脱壳技术 / Rootkit技术

## 研究第一大方面：缓冲区溢出和XSS

- 代表了**二进制软件**和**脚本软件**（包括绝大多数Web软件都是脚本软件开发的）两大技术方向
- 缓冲区溢出虽然比XSS底层，但是更难。XSS虽然比较上层，但是相对比较容易理解
  - 计算机科学的特点就是，越底层的东西，越难
  - 开发操作系统比开发app难多了

## 二进制软件

- 计算机的底层，是CPU直接执行在内存中的机器指令
- C和C++这类编程语言开发的软件，通过编译链接过程，把程序变成CPU可以直接执行的二进制指令
- 这类软件的一个特点也是不可避免的一点：需要直接操作内存(指针)
  - 内存是所有在运行态的软件及其数据保存的地方
  - 内存分为细小的单元，每个单元有一个唯一的地址
- 二进制软件安全问题的根源
  - 所有要访问数据，必须知道数据的地址，要保存新的数据，就必须分配内存，获得可用的地址
  - 地址也是数，如果不小心计算错误，就会访问到不该访问的数据，造成数据的泄露或者破坏。
- 二进制程序的编程，有很大的难度的原因
  - 二进制文件晦涩难懂
  - CPU只能执行二进制指令(是基于物理上的电路)，所以不可能设计得机制太复杂

## 脚本软件

- 在C和C++发展成熟以后，就有人去研究如何降低编程的难度，能否避免程序员编程时直接操作内存，把需要操作内存的地方，都封装起来，屏蔽在编程语言的内部 -> 发明了脚本语言
- 概念：用C和C++这样的二进制程序开一个软件来执行一种新的程序，就是用软件来模拟CPU工作
  - 由于软件的可定制性比CPU就高多了，可以想定义什么指令就定义什么指令
  - 对象与封装：把所有需要操作内存的东西，全部封闭在执行器内部，只给程序员接口，不给程序员操作内存的机会。比如把字符串封装为string**对象**。只能调用string.len()这样的方法来操作这个对象。这样就避免了由于编程不慎造成的内存相关问题，也降低了编程难度
- 如python、java、js、web浏览器这样的脚本程序的执行器都是二进制程序。
- 解决了内存相关问题，有引出了其他的问题
  - 用户输入问题。比如XSS出现的原因如下：web程序存在一种高交互性。web是互联网时代的软件的基本框架，所以会有用户提交数据。为了网页动态的需求，开发了网页的前端脚本，比如js，直接把脚本嵌入到网页中。浏览器只要发现了script标签，就去当做脚本来执行，把网页按照程序员的定制，变的丰富多彩，变得富于变化。但是，恰恰另外一种需求，就是UGC(User Generated Conten 用户生成内容)软件，也就是网页的内容来自于用户提交的内容，比如BBS、博客、微博，电商视频网站的用户评论，都会涉及到用户提交的内容在页面上呈现。**当用户提交的内容里含有脚本**，如果直接将用户提交的内容放在页面上，那么用户提交的内容中的脚本会不会被浏览器解析执行呢？那么一个用户提交了一个脚本就可以在这个页面的所有用户主机上执行呢？用户能提交程序执行了，怎么才能不保证这个程序不是恶意的呢？前端脚本，除了渲染页面元素这样的功能，还有获得用户的输入跳转页面到其他地址等等丰富的功能。

## 实验

### 编写一个简单的html文件

- 要求只要有一个表单，用户就可以在表单中输入数据，向服务器提交。
    
    ```html
    <html>
    <body>
    <form method="post">
            <input type="text">
            <button>提交</button>
        </form>
    </body>
    </html>
    ```

- 保存为index.html文件，用浏览器打开显示如下
  
  <img src="./readmeimg/2020-3-11-img/indexhtml.png" width=50%>

### 搭建一个web服务器

- 如果这个html是放在web服务器上，用户输入了数据，点击提交，浏览器就会把编辑框中的数据封装为一个POST请求，发现服务器。服务器会把这个数据发给后端脚本来处理。
- 通过定义form的属性来指明需要哪个脚本文件来处理
  - 比如PHP程序，他有一个POST超级变量，当用户提交了数据以后，对应的php脚本的post变量就是用户提交的数据
  - 假设服务器现在把用户提交的数据放在user_input.html的body标签中。然后保存在服务器文件的根目录中。当有网站的用户访问 http://xxxx.com/user_input.html 的时候，就会看到刚才那个表单用户提交的内容。当然实际的情况是这两个用户可能不是同一个用户，于是A用户提交的内容B用户就访问到了。
  - 当服务器脚本是原封不动的把用户输入的数据写到html里时，如果用户提交的数据中包括`<script>标签`，就会被执行。比如alert函数，弹出一个消息框，`<script>alert('xss attack')</script>`；比如给window.location.href赋值，能让用户莫名其妙的跳转到另外一个网站
- 最简单的实验环境
  - 在vscode中，安装一个php插件，然后编写一个简单的php脚本，调试运行这个脚本
  - F5 vscode会自动选择脚本运行的方式，把用户的表单输入写入到html文件
  - 通过浏览器访问这个文件html文件，这就是一个最简单的xss运行环境了。
- 实际的XSS漏洞可能很复杂，比如还会有数据库、登录等。另外，编程语言也不限于php，java、python也可以

### 问题

- 当编写的代码中出现中文，在网页上显示乱码
  - 编码问题，head标签里通过meta指定

# 教学网站

http://programtrace.com/

# 软件安全能力清单 

**要求：对照清单，上面所有的东西都掌握，一项项检测自己的能力**

</details>

<details>
<summary>2020-3-18 python搭建简单的http服务器端 + xss攻击</summary>
# 使用python内置的库开发一个基本的http服务器端

- [测试代码](./readmeimg/2020-3-18-img/httpserver.py)
- 使用python原生的cgi和http.server两个库运行的一个简单的http服务器程序
  - 因为没有使用第三方库，所有不需要使用pip安装依赖
  - 运行比较简单
  - 公共网关接口（Common Gateway Interface，CGI）是Web 服务器运行时外部程序的规范，按 CGI 编写的程序可以扩展服务器功能

## 讲解代码

```python
# -*- coding: utf-8 -*-

import sys
import cgi
from http.server import HTTPServer, BaseHTTPRequestHandler

class MyHTTPRequestHandler(BaseHTTPRequestHandler):
    field_name = 'a'
    form_html = \
        '''
        <html>
        <body>
        <form method='post' enctype='multipart/form-data'>
        <input type='text' name='%s'>
        <input type='submit'>
        </form>
        </body>
        </html>
        ''' % field_name

    def do_GET(self):
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        try:
            file = open("."+self.path, "rb")
        except FileNotFoundError as e:
            print(e)
            self.wfile.write(self.form_html.encode())
        else:
            content = file.read()
            self.wfile.write(content)

    def do_POST(self):
        form_data = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={
                'REQUEST_METHOD': 'POST',
                'CONTENT_TYPE': self.headers['Content-Type'],
            })
        fields = form_data.keys()
        if self.field_name in fields:
            input_data = form_data[self.field_name].value
            file = open("."+self.path, "wb")
            file.write(input_data.encode())

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(b"<html><body>OK</body></html>")


class MyHTTPServer(HTTPServer):
    def __init__(self, host, port):
        print("run app server by python!")
        HTTPServer.__init__(self,  (host, port), MyHTTPRequestHandler)


if '__main__' == __name__:
    server_ip = "0.0.0.0"
    server_port = 8080
    if len(sys.argv) == 2:
        server_port = int(sys.argv[1])
    if len(sys.argv) == 3:
        server_ip = sys.argv[1]
        server_port = int(sys.argv[2])
    print("App server is running on http://%s:%s " % (server_ip, server_port))

    server = MyHTTPServer(server_ip, server_port)
    server.serve_forever()

```

### 运行代码

- 使用`python httpserver.py`或者`py httpserver.py`
  - 使用vscode调试代码，在同目录下会自动生成一个`.vscode`的目录，目录下生成`launch.json`文件，配置如下
        
    ```json
    {
        // 使用 IntelliSense 了解相关属性。 
        // 悬停以查看现有属性的描述。
        // 欲了解更多信息，请访问: https://go.microsoft.com/fwlink/?linkid=830387
        "version": "0.2.0",
        "configurations": [
            {
                "name": "Python: 当前文件",
                "type": "python",
                "request": "launch",
                "program": "${file}",
                "console": "integratedTerminal"
            }
        ]
    }
    ```

- 在浏览器的访问网址为`127.0.0.1:8080/a.html`
  
  <img src="./readmeimg/2020-3-18-img/a-html.png" width=50%>

### 主要的父类与子类

#### http methods

- [http请求下的多种methods](https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods)
  - 通常使用得最多的，是 GET 和 POST
  - 直接在浏览器中输入链接，浏览器拿到地址以后，默认是采用 GET 方式向服务器发送请求，GET 方式最常见。
  - 表单使用的 post 方法提交数据 `<form method='post' enctype='multipart/form-data'>`
  - 通常来说，从服务器获取数据，使用 get 方法，向服务器提交数据，使用 post 方法
  - 其他的方法，在现在的web应用程序中，用到的很少

#### MyHTTPServer类

- MyHTTPServer类，是继承自原生的HTTPServer，重写 init 函数，增加了打印输出语言
- 然后直接调用父类 HTTPServer 的 init 函数传递了服务器运行需要的地址、端口等参数，我们的监听地址和端口是 0.0.0.0:8080

  ```py
  class MyHTTPServer(HTTPServer):
      def __init__(self, host, port):
          print("run app server by python!")
          HTTPServer.__init__(self,  (host, port), MyHTTPRequestHandler)
  ```

#### MyHTTPRequestHandler

- MyHTTPRequestHandler 类，这个是 HTTPServer 的回调，用来处理到达的请求，也就是 0.0.0.0:8080 上有任何的 HTTP 请求到达时，都会调用 MyHTTPRequestHandler来处理
  - MyHTTPRequestHandler 直接继承自 BaseHTTPRequestHandler
  - 重写了父类的 do_GET和do_POST两个方法
  - 这个HTTP请求的处理类是整个代码的主体，也是出问题的地方
  - 在 python 的 BaseHTTPRequestHandler 类中 ，do_XXX函数，就是处理对应的客户端请求的函数。代码指定了 MyHTTPRequestHandler 来处理 http 请求，那么当用 GET 方法请求，就会调用 do_GET，POST 方法请求，就会调用 do_POST函数
  - 浏览器所发送的数据包里包括请求类型， 在 http 的 headers里，会说明方法。
  - 这是python最基本的http 服务器的方式

### 其他处理

- 通常，一个静态的http服务器，这里的路径就是 http 服务器根目录下的文件，动态服务器可能是文件和参数，或者是对应其他服务器后台的处理过程
  - self.path 是这个请求的路径
  - 例如 http://127.0.0.1:8080/a.html 。其中 http://127.0.0.1:8080是协议服务器地址和端口。/a.html就是路径
  - 例如 http://127.0.0.1:8080/a.php?p1=x 。指定由 a.php 来处理这个请求，参数是 p1=x 。问号后面是参数，可以有多个
- 一般来说，如果读的文件不存在，应该返回404
  - self.send_response(200) 按照协议应该是404
  - 则这里的处理为 如果指定的文件不存在，还是返回200，表示请求路径是正确的，可以处理，然后返回一个默认的页面。这个页面是 form_html的变量，在FileNotFoundError异常处理过程中写回
  - self.wfile 和 self.rfile 对应 http 响应和请求的 body 部分
- GET处理完成以后，浏览器就拿到了 200 状态的  "Content-type" 为 "text/html" 的 form_html
- 在浏览器刷新是重复上一次的POST请求，所以会提示是否要重新提交表单

### 调试运行

- 在 def do_GET 下断点，刷新浏览器，代码就会断点命中中断。
- 结合浏览器，抓包看看 http 请求和响应的数据格式 （用抓包器或者浏览器的调试模式观察）

#### 浏览器调试模式

- 打开浏览器的调试模式(chrome > 菜单 > 更多工具 > 开发者工具)
- 在sources这个标签下看到服务器向浏览器返回的数据，即 form_html 变量
  - 这一段 html 浏览器渲染出来，就是那个带一个编辑框的表单- 表单指定了使用post方式向服务器提交数据
  
    <img src="./readmeimg/2020-3-18-img/sourcetab.png">

- 在network tab里可以看到完整的请求响应过程
  
  <img src="./readmeimg/2020-3-18-img/networktab.png">

  - 完整的网络数据，其中 header 里就说了 GET 或者 POST 、返回的状态码200等等
    
    <img src="./readmeimg/2020-3-18-img/headers.png">

- 在表单中填入数据，点提交按钮，然后服务器的 do_POST 函数被调用。通过 cgi.FieldStorage解析了客户端提交的请求，原始的请求的头部在self.headers。body部分在self.rfile。解析完成以后放到 form_data变量里，其中 form_data['field_name'].value 是在编辑框中填入的数据
  ```py
  form_data = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={
                'REQUEST_METHOD': 'POST',
                'CONTENT_TYPE': self.headers['Content-Type'],
            })
  ```

### 使用场景

- 通常，一个服务器会根据业务逻辑处理用户提交的数据，比如用户发表的商品评论，比如在线教学系统中填入的作业一般会写入数据库。但是这些数据，在某些情况下又会被显示出来，比如其他用户查看别人的商品评论的时候，比如老师查看学生的作业时。
- 为了模拟这个过程，简化了一下，没有用户系统，也没有数据库，直接写入了 path 对应的文件。
- 处理用户提交，写入文件
  ```py
  fields = form_data.keys()
  if self.field_name in fields:
      input_data = form_data[self.field_name].value
      file = open("."+self.path, "wb")
      file.write(input_data.encode())
  ```
  - fields = form_data.keys()是获取表单中的键值对，因此使用.value得到输入的值：这里获得是对应的是form中input的name `<input type='text' name='%s'>`
  - 表单以变量名变量值的方式组织，input的name相当于变量名，填入的数据就是变量值
  - python的cgi.FieldStorage将form组织为python的dict数据类型，所以可以通过  form_data['field_name'].value 获得所填入的数据
- 如果写入成功，就返回一个 200 状态的 OK 
  ```py
  self.send_response(200)
  self.send_header("Content-type", "text/html")
  self.end_headers()
  self.wfile.write(b"<html><body>OK</body></html>")
  ``` 

## 漏洞

- 如果向网页中填入了 123 ，那么123被写入了a.html文件。执行完成后，同目录下会多一个a.html，内容为123。然后下次再访问 http://127.0.0.1:8080/a.html 时，在浏览器地址栏里回车。由于这个时候a.html已经存在了，所以是运行的部分是
  ```py
  else:
      content = file.read()
      self.wfile.write(content)
  ```
  - 会直接把文件内容会写给浏览器
  - 这里是在简化模拟用户提交数据 > 存入数据 > 其他用户获取这个数据的过程
  
  <img src="./readmeimg/2020-3-18-img/httpserver1.gif">

- XSS漏洞
  - 再访问一个不存在的页面，比如b.html，又会出现那个默认的form。这时输入`<html><body><script>alert('XSS')</script></form></body></html>`，然后在访问b.html的时候，整个页面被载入 script 在浏览器上执行，也就是**用户提交的数据被执行了**

    <img src="./readmeimg/2020-3-18-img/httpserver-xss.gif">

  - 重新开一个页面，在c.html中填入`<html><body><script>window.location.href='http://by.cuc.edu.cn'</script></form></body></html>`。下次再访问c.html的时候。页面跳转了。`window.location.href='http://by.cuc.edu.cn'` 这段脚本的功能是实现了跳转
        
- 理论上，任何的js都是可以被执行的。js可以实现非常丰富的功能。比如可以骗取用户扫码、支付，实际到黑客的账户。如果是没有基本防御措施的网站，这段会被放进服务器数据库里，然后别人提交了数据就自动跳转到这个网站。比如有一个商品A，用户在评论里输入了一段js代码。如果服务器不做处理直接保存。后面的用户访问商品A、看评论，前一个用户输入的代码就会在其他用户的页面上执行。

### Furthermore

- 如果大家在浏览器中访问 http://127.0.0.1:8080/httpserver.py ，则在sources中显示全部完整的源代码。由于服务器没有做任何过滤，只要是存在的文件，就发送给客户端
  - 现在黑客可以知道我整个后台的逻辑了。
  - 如果还有一些配置文件，比如数据库地址和访问口令等。那就更严重了
  - 更严重的是，黑客甚至可以注入后端代码。由于我们是回写到文件，可以构造一个http post请求，把httpserver.py文件改写了。但是构造这个请求用浏览器就不行了，需要采用curl等更基础的工具裸写post请求发送给服务器的
- 在调试工具的 elements tab，由于后台只处理名为a的表单项写入文件，所以我们需要把input的把 name="%s" 改为 name="a" 再提交。改为以后，同时在提交框中输入‘hahaha’提交。此时httpserver.py，它变为'hahaha'，只是注入一个hahaha 服务器就挂了，再也跑不起来了。
  
  <img src="./readmeimg/2020-3-18-img/httpserver-xss3.gif">

- 所以，这是一个及其简单，但是漏洞百出的web服务器。这就是不做任何过滤，直接写入数据的危害。

## 参考资料

- [http methods](https://developer.mozilla.org/en-US/docs/Web/HTTP/Methods)
</details>