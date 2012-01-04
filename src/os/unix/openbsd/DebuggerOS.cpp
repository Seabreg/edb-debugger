/*
Copyright (C) 2006 - 2011 Evan Teran
                          eteran@alum.rit.edu

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "Debugger.h"
#include "IDebuggerCore.h"
#include "MemoryRegions.h"

#include <QtDebug>
#include <QTextStream>
#include <QFile>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/exec.h>
#include <sys/proc.h>
#include <sys/sysctl.h>

#include <kvm.h>
#include <uvm/uvm.h>
#include <uvm/uvm_amap.h>

#include <fcntl.h>
#include <unistd.h>

//------------------------------------------------------------------------------
// Name: primary_code_region()
// Desc:
//------------------------------------------------------------------------------
EDB_EXPORT MemRegion edb::v1::primary_code_region() {

	const QString process_executable = get_process_exe();

	memory_regions().sync();

	const QList<MemRegion> r = memory_regions().regions();
	Q_FOREACH(const MemRegion &region, r) {
		if(region.executable() && region.name == process_executable) {
			return region;
		}
	}
	return MemRegion();
}

//------------------------------------------------------------------------------
// Name: loaded_libraries()
// Desc:
//------------------------------------------------------------------------------
QStringList edb::v1::loaded_libraries() {
	qDebug("TODO: implement edb::v1::loaded_libraries");
	QStringList ret;

	memory_regions().sync();

	const QList<MemRegion> r = memory_regions().regions();
	Q_FOREACH(const MemRegion &region, r) {
	}



	return ret;
}

//------------------------------------------------------------------------------
// Name: get_parent_pid(edb::pid_t pid)
// Desc:
//------------------------------------------------------------------------------
edb::pid_t edb::v1::get_parent_pid(edb::pid_t pid) {

	edb::pid_t ret = 0;
	char errbuf[_POSIX2_LINE_MAX];
	if(kvm_t *kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf)) {
		int rc;
		struct kinfo_proc2 *const proc = kvm_getproc2(kd, KERN_PROC_PID, pid, sizeof(struct kinfo_proc2), &rc);
		ret = proc->p_ppid;
		kvm_close(kd);
	}
	return ret;
}

//------------------------------------------------------------------------------
// Name: get_process_exe()
// Desc:
//------------------------------------------------------------------------------
QString edb::v1::get_process_exe() {
	QString ret;

	if(debugger_core != 0) {
		// TODO: assert attached!
		const edb::pid_t pid = debugger_core->pid();

		char errbuf[_POSIX2_LINE_MAX];
		if(kvm_t *kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf)) {

			int rc;
			struct kinfo_proc2 *const proc = kvm_getproc2(kd, KERN_PROC_PID, pid, sizeof(struct kinfo_proc2), &rc);

			char p_comm[KI_MAXCOMLEN] = "";
			memcpy(p_comm, proc->p_comm, sizeof(p_comm));


			kvm_close(kd);
			return p_comm;
		}

	}

	return ret;
}

//------------------------------------------------------------------------------
// Name: get_process_cwd()
// Desc:
//------------------------------------------------------------------------------
QString edb::v1::get_process_cwd() {
	qDebug("TODO: implement edb::v1::get_process_cwd");

	QString ret;
	if(debugger_core != 0) {
		// TODO: assert attached!
		const edb::pid_t pid = debugger_core->pid();
	}
	return ret;
}

//------------------------------------------------------------------------------
// Name: get_process_args()
// Desc:
//------------------------------------------------------------------------------
QStringList edb::v1::get_process_args() {
	QStringList ret;
	if(debugger_core != 0) {

		// TODO: assert attached!
		const edb::pid_t pid = debugger_core->pid();
		char errbuf[_POSIX2_LINE_MAX];
		if(kvm_t *kd = kvm_openfiles(NULL, NULL, NULL, O_RDONLY, errbuf)) {
			int rc;
			if(struct kinfo_proc *const proc = kvm_getprocs(kd, KERN_PROC_PID, pid, &rc)) {
				char **argv = kvm_getargv(kd, proc, 0);
				char **p = argv;
				while(*p != 0) {
					ret << *p++;
				}
			}
			kvm_close(kd);
		}

	}
	return ret;
}
