
import * as os from 'quark/os'
import { LOG,Mv } from './tool'

export default async function(_: any) {
	LOG('\nOs:\n');
	Mv(os, 'version', [])
	Mv(os, 'brand', [])
	Mv(os, 'model', [])
	Mv(os, 'info', [])
	Mv(os, 'languages', [])
	Mv(os, 'isWifi', [])
	Mv(os, 'isMobile', [])
	Mv(os, 'networkInterface', [])
	Mv(os, 'isAcPower', [])
	Mv(os, 'isBattery', [])
	Mv(os, 'batteryLevel', [])
	Mv(os, 'memory', [])
	Mv(os, 'usedMemory', [])
	Mv(os, 'availableMemory', [])
	Mv(os, 'cpuUsage', [])
}