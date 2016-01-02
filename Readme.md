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
  "ch_data": channel_data[]
}
```

Subscribed Daylite Messages
==========================

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
    cd build
    cmake ..
    make

Installation
=============

    cd /path/to/boyd
    cd build
    make install
