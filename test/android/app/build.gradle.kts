import com.android.build.gradle.internal.tasks.getTestOnlyNativeLibs
import org.gradle.internal.declarativedsl.parsing.main

plugins {
	alias(libs.plugins.android.application)
}

var isCmake = true

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
					//"x86_64",
					"arm64-v8a",
					//"armeabi-v7a",
					//"x86",
				)
				//arguments += "-DANDROID_TOOLCHAIN=gcc"
				//arguments += "-DCMAKE_BUILD_TYPE=Debug"
				//arguments += "-DANDROID_STL=c++_shared"
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
	buildFeatures {
		viewBinding = true
	}

	externalNativeBuild {
		if (isCmake)
			cmake {
				path = file("CMakeLists.txt")
				version = "3.22.1"
			}
	}
	sourceSets {
		getByName("main") {
			java.srcDirs(
				"src/main/java",
				"../../../src/platforms/android",
			)
			jniLibs.srcDirs(
				"src/main/jniLibs",
				if (isCmake)
					"../../../out/jniLibs"
				else
					"../../../out/qkmake/product/android/jniLibs",
			)
			assets.srcDirs(
				"../../jsapi/out/jsapi",
				//"/Users/louis/Workspace/kkkk/out/small"
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
	//implementation(fileTree(mapOf("dir" to "../../../out/qkmake/product/android/libs", "include" to listOf("*.jar"))))
}
