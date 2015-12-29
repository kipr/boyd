Boyd
======


Published Daylite Messages
==========================

`camera/frame_data`
-------------------

```
"msg": {
  "format": string,
  "width": uint32,
  "height": uint32,
  "data": uint8[],
  "blobs": blob[]
}
```

Subscribed Daylite Messages
==========================

`camera/heartbeat`
-------------------

```
"msg": {
  "frames": bool,
  "blobs": bool
}
```
The heartbeat includes flags for indicating whether the publisher needs frame and/or blob data.

`camera/settings`
-------------------

```
"msg": {
  "width": uint16,
  "height": uint16,
  "config_base_path": string,
  "config_name": string
}
```

Compilation
===============

    cd /path/to/boyd
    mkdir build
    cmake ..
    make

Installation
=============

    cd /path/to/boyd
    make install
