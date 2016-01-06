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

`camera/settings`
-------------------

```
"msg": {
  "width": uint16,
  "height": uint16,
  "maxNumBlobs": uint16,
  "config_base_path": string,
  "config_name": string,
  "camera_configs": camera_config[]
}
```

Subscribed Daylite Messages
==========================

`camera/set_settings`
-------------------

```
"msg": {
  "width": uint16,
  "height": uint16,
  "maxNumBlobs": uint16,
  "config_base_path": string,
  "config_name": string,
  "camera_configs": camera_config[]
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
