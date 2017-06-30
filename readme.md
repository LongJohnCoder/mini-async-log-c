Description
===========

A C11 low-latency producer wait-free (when using Thread Local Storage)
asynchronous data logger with type-safe strings.

Based on the lessons learned on its older C++ counterpart mini-async-log.

Features
========

- Very high performance.

- Various memory (log entry) sources: TLS, common bounded "heap" and the C
  library heap. All can be used at the same time.

- Type-safe format strings. Achieved through C11 type-generic expressions and
  (unfortunately) preprocessor abusing.

- C++ compatible/compilable.

- No consumer logger thread ownership. The client application can run the
  logger main loop from an existing (maybe shared for other purposes) thread.

- Basic security features: Log entry rate limiting and newline removal (to
  be used if bindings to other languages are written).

- Extensible log destinations (sinks).

- Compile-time removable severities.

- Very decent test coverage.

Log format strings
==================

The format strings have this form:

> "this is value 1: {} and this is value 2: {}"

These are like printf strings where the length related specifiers are detected
automatically. So you never need to specify e.g hh, llu, d, etc.

All the strings inside the braces will be passed internally to print as format
modifiers, e.g.

> "this is a fixed 8 char width zero padded integer: {08}"

You can use all the other non-length specifiers:

> "hex 8 char width zero padded integer: {08x}"

The new "W" specifier is replaced by the maximum character count that an
integer can allocate (3 for 1 byte, 5 for 2 bytes, 10 for 4 bytes and 20 for 8
bytes):

> "full width zero padded integer: {0W}"

The new "N" specifier is replaced by the number of nibbles of an integer:

> "full width hex zero padded integer: {0Nx}"

The "*" width and ".*" precision specifiers are unsupported.

The opening brace '{' is escaped by doubling it. The close brace doesn't need
escaping:

> "Escaped open brace: {{"

Build
==================

Meson 0.41 is used. In Ubuntu:

> sudo -H pip3 install meson