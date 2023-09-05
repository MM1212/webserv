import cgi
import cgitb; cgitb.enable()

print("Content-Type: text/html\n")

# get the form data
form = cgi.FieldStorage()

a = form.getvalue("a")
b = form.getvalue("b")

# calculate the sum
result = int(a) + int(b)

message = "The sum of %s + %s is %s" % (a, b, result)

print("""\
<html><body>
<p>%s</p>
</body></html>
""" % (message))