# create this nodejs script from scratch
# CGI SCRIPT add.js 
# a form will trigger this script 
# this fields are: 
# - a 
# - b 
# - file<image/jpeg> 
# output will be the result of the sum between a and b 
# and a image displaying the file contents

import cgi, os, base64
import cgitb; cgitb.enable()

# get the form data
form = cgi.FieldStorage()

PATH_INFO = os.environ.get('PATH_INFO', './')

if not os.path.exists(PATH_INFO):
  os.makedirs(PATH_INFO)

# Generator to buffer file chunks
def fbuffer(f, chunk_size=10000):
    while True:
        chunk = f.read(chunk_size)
        if not chunk: break
        yield chunk

# A nested FieldStorage instance holds the file
fileitem = form['file']

# Test if the file was uploaded
if fileitem.filename:

    # strip leading path from file name to avoid directory traversal attacks
    fn = os.path.basename(fileitem.filename)
    path = os.path.realpath(os.path.join(PATH_INFO, fn))
    f = open(path, 'wb', 10000)

    # Read the file in chunks
    for chunk in fbuffer(fileitem.file):
      f.write(chunk)
    f.close()
    message = 'The file "' + fn + '" was uploaded successfully\n<img src="%s" />' % (path,)

else:
    message = 'No file was uploaded'

print( """\
Content-Type: text/html\n
<html><body>
<p>%s</p>
</body></html>
""" % (message,))