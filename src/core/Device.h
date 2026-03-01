/*
	Device.h is part of DualSenseWindows
	https://github.com/Ohjurot/DualSense-Windows

	Contributors of this file:
	11.2020 Ludwig Füchsl
    02.2026 Thanasis Petsas

	Licensed under the MIT License (To be found in repository root directory)
*/
#pragma once

namespace DS5W {
	/// <summary>
	/// Enum for device connection type
	/// </summary>
	typedef enum class _DeviceConnection : unsigned char {
		/// <summary>
		/// Controler is connected via USB
		/// </summary>
		USB = 0,

		/// <summary>
		/// Controler is connected via bluetooth
		/// </summary>
		BT = 1,
	} DeviceConnection;

	/// <summary>
	/// Struckt for storing device enum info while device discovery
	/// </summary>
	typedef struct _DeviceEnumInfo {
		/// <summary>
		/// Encapsulate data in struct to (at least try) prevent user from modifing the context
		/// </summary>
		struct {
			/// <summary>
			/// Path to the discovered device
			/// </summary>
			wchar_t path[260];

			/// <summary>
			/// Connection type of the discoverd device
			/// </summary>
			DeviceConnection connection;
		} _internal;
	} DeviceEnumInfo;

	/// <summary>
	/// Device context
	/// </summary>
	typedef struct _DeviceContext {
		/// <summary>
		/// Encapsulate data in struct to (at least try) prevent user from modifing the context
		/// </summary>
		struct {
			/// <summary>
			/// Path to the device
			/// </summary>
			wchar_t devicePath[260];

			/// <summary>
			/// Handle to the open device
			/// </summary>
			void* deviceHandle;

			/// <summary>
			/// Connection of the device
			/// </summary>
			DeviceConnection connection;

			/// <summary>
			/// Current state of connection
			/// </summary>
			bool connected;

			/// <summary>
			/// HID Input buffer (will be allocated by the context init function)
			/// </summary>
			unsigned char hidBuffer[547];

            /// <summary>
            /// Length in bytes of the HID input report for the currently opened
            /// device interface (as reported by HIDP_CAPS::InputReportByteLength).
            /// This may differ between controller models (e.g. DualSense vs Edge).
            /// </summary>
            unsigned short inputReportLen;

            /// <summary>
            /// Length in bytes of the HID output report for the currently opened
            /// device interface (as reported by HIDP_CAPS::OutputReportByteLength).
            /// Must be used when writing output reports instead of hardcoded values,
            /// since different DualSense models expose different USB report sizes.
            /// </summary>
            unsigned short outputReportLen;

            /// <summary>
            /// Length in bytes of the HID feature report for the currently opened
            /// device interface (as reported by HIDP_CAPS::FeatureReportByteLength).
            /// Used for feature report communication such as Bluetooth initialization.
            /// </summary>
            unsigned short featureReportLen;

		}_internal;
	} DeviceContext;
}
