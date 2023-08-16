/// @ts-check

console.log('Content-Type: text/html');

const { URLSearchParams } = require('url');

const params = new URLSearchParams(process.env.QUERY_STRING);
try {

  const a = Number(params.get('a'));
  if (isNaN(a))
    throw new Error('a is not a number');
  const b = Number(params.get('b'));
  if (isNaN(b))
    throw new Error('b is not a number');

  console.log('');
  console.log(`<h1>add.js</h1>`);
  console.log(`<output>${a} + ${b} = ${a + b}</output>`);
}
catch (err) {
  console.error(err);
  console.log('Status: 500');
  console.log('');
  console.log(`<h1>add.js</h1>`);
  console.log(`<p>${err.message}</p>`);
}