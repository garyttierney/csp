# Introduction

CSP is a SELinux policy language built on the High Level Language
infrastructure.  It's primary goal is providing a less verbose language that
preserves the semantics of CIL and gives policy authors a better development
experience.

The basis of CSP is taking CIL and replacing it's Lisp style S-expressions with
a C-style expression language that incorporates syntax from the existing kernel
policy language.  This gives us a language that is instantly familiar to both
kernel policy language and CIL policy writers.

# Syntax

As previously mentioned, CSP aims to preserves the semantics of CIL while
improving on the syntax.  This is demonstrated in how security attributes are
declared and represented in both languages:

CSP
```csp
user myuser;
role myrole;
type mytype;
level low = s0;
level high = s0:c0.255;
level_range low_high = low - high;
context myctx = myuser:myrole:mytype:low_high;
```

CIL (compiler output):
```cil
(user myuser)
(role myrole)
(type mytype)
(level low (s0))
(level high (s0 (range c0 c255)))
(levelrange low_high (low high))
(context myctx (myuser myrole mytype low_high))
```

The examples are almost syntactically identical.  CSP replaces the parenthesis
with semi-colon delimited statements and represents declaration initializers as
assignment expressions.  The attributes are also represented using the same
syntax as the kernel policy language and libselinux.

CIL also introduced the ability to have _block_s, or _namespace_s.  These
are similar to refpolicy _template_s and provide an inheritance mechanism,
which provides the policy author with a method to group common sections of policy.

CSP
```csp
abstract block common_ns {
  type b;
}

block ns inherits_from common_ns {
  type a;
}

type_alias alias_name = ns.b;
```

CIL (compiler output):
```cil
(block common_ns
  (type b)
)

(block ns
  (blockinherit common_ns)
  (type a)
)

(typealias alias_name ns.b)
