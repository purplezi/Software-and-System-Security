# -*- coding: utf-8 -*-

import sqlite3

conn = sqlite3.connect('edu_admin.db')

c = conn.cursor()

# Create teacher table 
c.execute('''CREATE TABLE teacher(courseid,studentid,grade)''')

# Insert a row of data
# c.execute("INSERT INTO teacher VALUES ('1','1','90')")

# Create 

# Save (commit) the change
conn.commit()

# We can also close the connection if we are done with it.
# Just be sure any changes have been committed or they will be lost.
conn.close()