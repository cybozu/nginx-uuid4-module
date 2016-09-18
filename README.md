nginx-uuid4-module
==================

`nginx-uuid4-module` provides a directive to set version-4 UUIDs for variables.

This module randomly generates UUID by Mersenne twister.
You can use the UUID as an unique identifier for an HTTP request.

NOTE: You should not treat these UUIDs as secret credentials because an attacker
can predict subsequent UUIDs by observing past UUIDs.


Example configuration
---------------------

```
http {
    # Set a random UUID to $request_id.
    uuid4 $request_id;
    # Output the $request_id to our access log.
    log_format combined "$remote_addr ... $http_user_agent $request_id";
}
```

In this example, `$request_id` contains a UUID string such as
`b4c3a954-8403-4da2-9f7a-aecc9a98b28f`.


Directives
----------

### uuid4

Syntax: `uuid4 VARIABLE`  
Context: http, server, location

Generates a version-4 UUID and assigns it to the specified variable.


Install
-------

Specify `--add-module=/path/to/nginx-uuid4-module` when you run `./configure`.

Example:

```
./configure --add-module=/path/to/nginx-uuid4-module
make
make install
```


License
-------

- `nginx-uuid4-module` is licensed under [the BSD 2-Clause License](LICENSE).
- Mersenne twister (`mt19937`) is licensed under the BSD 3-Clause License.
