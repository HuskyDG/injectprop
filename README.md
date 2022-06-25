# injectprop

Inject Android system properties, allow you to pass SafetyNet on Custom ROM without Magisk.

## Usage

- Download zip from [here](https://github.com/HuskyDG/injectprop/releases/tag/latest)  and extract, configure information in `system/bin/injectprop.sh`, repack as zip and flash it through Custom Recovery (TWRP).

- Inject or hide sensitive properties:

```
injectprop --sensitive-props
```

- Add or modify system properties
```
injectprop [NAME] [VALUE]
```
