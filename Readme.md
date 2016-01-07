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

# channel_data
"msg": {
  "blobs": blob[]
}

# blob
"msg": {
  "centroidX": uint16,
  "centroidY": uint16,
  "bBoxX": uint16,
  "bBoxY": uint16,
  "bBoxWidth": uint16,
  "bBoxHeight": uint16,
  "confidence": uint16
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

# camera_config
"msg": {
  "config_name": string,
  "channels": channel_config[]
}

# channel_config
"msg": {
  "channel_name": string,
  "th": uint8,
  "ts": uint8,
  "tv": uint8,
  "bh": uint8,
  "bs": uint8,
  "bv": uint8
}
```

Subscribed Daylite Messages
==========================

`camera/set_settings`
-------------------

See format above for `camera/settings`.

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
