import org.gradle.internal.declarativedsl.parsing.main

plugins {
	alias(libs.plugins.android.application)
}

android {
	namespace = "org.quark.test"
	compileSdk = 35

	defaultConfig {
		applicationId = "org.quark.test"
		minSdk = 28
		targetSdk = 35
		versionCode = 1
		versionName = "1.0"

		testInstrumentationRunner = "androidx.test.runner.AndroidJUnitRunner"
		externalNativeBuild {
			cmake {
				cppFlags += "-std=c++14"
				abiFilters += setOf(
					//"armeabi-v7a",
					"arm64-v8a",
					//"x86",
					//"x86_64",
				)
				//arguments += "-DANDROID_TOOLCHAIN=gcc"
				//arguments += "-DCMAKE_BUILD_TYPE=Debug"
			}
		}
	}

	buildTypes {
		release {
			isMinifyEnabled = false
			proguardFiles(
				getDefaultProguardFile("proguard-android-optimize.txt"),
				"proguard-rules.pro"
			)
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
	buildFeatures {
		viewBinding = true
	}
	sourceSets {
		getByName("main") {
			java.srcDirs(
					"src/main/java",
					"../../../src/platforms/android",
			)
			jniLibs.srcDirs(
				"src/main/jniLibs",
				"../../../out/jniLibs",
			)
			assets.srcDirs(
				"../../jsapi/out",
			)
		}
	}
}

dependencies {
	implementation(libs.appcompat)
	implementation(libs.material)
	implementation(libs.constraintlayout)
	testImplementation(libs.junit)
	androidTestImplementation(libs.ext.junit)
	androidTestImplementation(libs.espresso.core)
}