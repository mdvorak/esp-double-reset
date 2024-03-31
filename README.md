# double_reset

[![build](https://github.com/mdvorak/esp-double-reset/actions/workflows/build.yml/badge.svg)](https://github.com/mdvorak/esp-double-reset/actions/workflows/build.yml)

Detect double reset, which can be used to place program in special reconfiguration mode, like entering Wi-Fi credentials.

## Usage

To reference this library by your ESP-IDF project, add it as a component

```shell
idf.py add-dependency "mdvorak/double_reset^2.0.1"
```

To use it as a platformio library, add repository to `lib_deps`:

```ini
[env]
lib_deps =
    https://github.com/mdvorak/esp-double-reset.git#v2.0.1
```

Note that when used as platformio library, `Kconfig` is not available. Either add [Kconfig](./Kconfig) contents to 
your `Kconfig.projbuild`, or set `DOUBLE_RESET_DEFAULT_TIMEOUT` manually, via

```ini
[env]
build_flags = -D DOUBLE_RESET_DEFAULT_TIMEOUT=3000
```

### Example

```c
#include <double_reset.h>

void app_main() 
{
	// Initialize NVS
	esp_err_t ret = nvs_flash_init();
	if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
	{
		ESP_ERROR_CHECK(nvs_flash_erase());
		ret = nvs_flash_init();
	}
	ESP_ERROR_CHECK(ret);

	// Check double reset
	// NOTE this should be called as soon as possible, ideally right after nvs init
	bool reconfigure = false;
	ESP_ERROR_CHECK(double_reset_start(&reconfigure, 10000));

	if (reconfigure)
	{
		ESP_LOGI(TAG, "double reset detected!");
	}

	// Setup complete
	ESP_LOGI(TAG, "started");
}
```

For working example, see [example/main.cpp](example/main.cpp).

## Development

Prepare [ESP-IDF development environment](https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/index.html#get-started-get-prerequisites)
.

Configure example application with

```
cd example/
idf.py menuconfig
```

Flash it via (in the example dir)

```
idf.py build flash monitor
```

As an alternative, you can use [PlatformIO](https://docs.platformio.org/en/latest/core/installation.html) to build and
flash the example project.
