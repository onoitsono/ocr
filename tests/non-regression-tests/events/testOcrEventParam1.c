/* Copyright (c) 2012, Rice University

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are
met:

1.  Redistributions of source code must retain the above copyright
     notice, this list of conditions and the following disclaimer.
2.  Redistributions in binary form must reproduce the above
     copyright notice, this list of conditions and the following
     disclaimer in the documentation and/or other materials provided
     with the distribution.
3.  Neither the name of Intel Corporation
     nor the names of its contributors may be used to endorse or
     promote products derived from this software without specific
     prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "ocr.h"

#define FLAGS 0xdead

u8 task_for_edt ( u32 paramc, u64 * params, void* paramv[], u32 depc, ocrEdtDep_t depv[]) {
    printf("In the task_for_edt with value %d\n", *((int *)(paramv[0])));
    assert(paramc == 1);
    assert(params[0] == sizeof(int));
    assert(*((int *)(paramv[0])) == 32);
    // This is the last EDT to execute, terminate
    ocrFinish();
    return 0;
}

int main (int argc, char ** argv) {
    ocrEdt_t fctPtrArray [1];
    fctPtrArray[0] = &task_for_edt;
    ocrInit(&argc, argv, 1, fctPtrArray);

    // Current thread is '0' and goes on with user code.
    ocrGuid_t depv[1];
    ocrGuid_t event_guid;
    ocrEventCreate(&event_guid, OCR_EVENT_STICKY_T, true);
    depv[0] = event_guid;

    // Creates the EDT
    u32 paramc = 1;
    u64 params[1];
    params[0] = sizeof(int);
    int * paramv = (int *) malloc(sizeof(int));
    paramv[0] = 32;
    ocrGuid_t edt_guid;

    // 'depv' stores dependencies, so no need to call
    // ocrAddDependence later on to register events.
    ocrEdtCreate(&edt_guid, task_for_edt, paramc, params, (void**) &paramv, 0, 1, depv, NULL_GUID);

    int *k;
    ocrGuid_t db_guid;
    ocrDbCreate(&db_guid,(void **) &k,
            sizeof(int), /*flags=*/FLAGS,
            /*location=*/NULL,
            NO_ALLOC);
    *k = 42;

    ocrEventSatisfy(event_guid, db_guid);

    ocrEdtSchedule(edt_guid);

    ocrCleanup();

    return 0;
}
