# -*- coding: utf-8 -*-

import sys
import cgi
from http.server import HTTPServer, BaseHTTPRequestHandler
import sqlite3

def insert_into_db(course_id,student_id,grade):
    print("database")
    conn = sqlite3.connect('./edu_admin.db')
    c = conn.cursor()
    try:
        c.execute("select * from teacher where courseid = %s and studentid = %s " % (course_id,student_id))
        jud = c.fetchall()
        if len(jud) == 0:
            c.execute("INSERT INTO teacher VALUES (%s,%s,%s)" %(course_id,student_id,grade))
            # Save (commit) the change
            conn.commit()
    except:
        print("already exist")
        conn.rollback()
    conn.close()

class MyHTTPRequestHandler(BaseHTTPRequestHandler):
    course_id = 'course_id'
    student_id = 'student_id'
    grade = 'grade'
    form_html = \
        '''
        <html>
        <body>
        <form method='post' enctype='multipart/form-data'>
        course id:</br>
        <input type='text' name='%s'></br>
        student id:</br>
        <input type='text' name='%s'></br>
        grade:<br>
        <input type='text' name='%s'></br>
        <input type='submit'>
        </form>
        </body>
        </html>
        ''' % (course_id,student_id,grade)

    def do_GET(self):
        print("get success")
        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        print(self.path)
        self.wfile.write(self.form_html.encode())
        # try:
        #     file = open("."+self.path, "rb")
        #     print("success")
        # except FileNotFoundError as e:
        #     print(e)
        #     self.wfile.write(self.form_html.encode())
        # else:
        #     content = file.read()
        #     self.wfile.write(content)

    def do_POST(self):
        print("post success")
        form_data = cgi.FieldStorage(
            fp=self.rfile,
            headers=self.headers,
            environ={
                'REQUEST_METHOD': 'POST',
                'CONTENT_TYPE': self.headers['Content-Type'],
            })
        fields = form_data.keys()
        if self.course_id in fields and self.student_id in fields and self.grade in fields:
            course = form_data[self.course_id].value
            print(course)
            student = form_data[self.student_id].value
            print(student)
            gra = form_data[self.grade].value
            print(gra)
            # file = open("."+self.path, "wb")
            # file.write("course_id: %s student_id: %s grade: %s \n" % (course,student,gra))
            
            # database
            insert_into_db(course,student,gra)

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write(b"<html><body>OK</body></html>")

# MyHTTPServer类，是继承自原生的HTTPSever，55行。
class MyHTTPServer(HTTPServer):
    def __init__(self, host, port):
        print("run app server by python!")
        HTTPServer.__init__(self,  (host, port), MyHTTPRequestHandler)

# 61行，是程序入口
if '__main__' == __name__:
    server_ip = "127.0.0.1"
    server_port = 8080
    if len(sys.argv) == 2:
        server_port = int(sys.argv[1])
    if len(sys.argv) == 3:
        server_ip = sys.argv[1]
        server_port = int(sys.argv[2])
    print("App server is running on http://%s:%s " % (server_ip, server_port))

    server = MyHTTPServer(server_ip, server_port)
    server.serve_forever()
