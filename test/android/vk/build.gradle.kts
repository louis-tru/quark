plugins {
	id("com.android.application")
}

android {
	namespace = "org.quark.test_vk"
	compileSdk = 35

	defaultConfig {
		applicationId = "org.quark.test_vk"
		minSdk = 28
		targetSdk = 35
		versionCode = 1
		versionName = "1.0"

		externalNativeBuild {
			cmake {
				cppFlags += listOf("-std=c++14", "-Wall", "-Wextra")
				abiFilters += "arm64-v8a"
			}
		}
	}

	compileOptions {
		sourceCompatibility = JavaVersion.VERSION_11
		targetCompatibility = JavaVersion.VERSION_11
	}

	externalNativeBuild {
		cmake {
			path = file("CMakeLists.txt")
			version = "3.22.1"
		}
	}
}
