### To generate certificate
```sh
openssl req -x509 -newkey rsa:2048 -nodes -keyout server_key.pem -out server_cert.pem -days 365
```

### OR

```sh
# Generate a Private Key
openssl genrsa -out server_key.pem 2048
# openssl req -new -key server_key.pem -out server_csr.pem
openssl req -new -key server_key.pem -out server_csr.pem
# Generate Self-Signed Certificate (Valid for 3 years)
openssl x509 -req -in server_csr.pem -signkey server_key.pem -out server_cert.pem -days 1095

