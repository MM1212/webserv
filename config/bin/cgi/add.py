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
    f = open('uploads/' + fn, 'wb', 10000)

    # Read the file in chunks
    for chunk in fbuffer(fileitem.file):
      f.write(chunk)
    f.close()
    message = 'The file "' + fn + '" was uploaded successfully\n<img src="uploads/%s" />' % (fn,)

else:
    message = 'No file was uploaded'

print( """\
Content-Type: text/html\n
<html><body>
<p>%s</p>
</body></html>
""" % (message,))