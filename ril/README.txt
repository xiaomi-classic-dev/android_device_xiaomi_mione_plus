MiOne CAF RIL v6 adapter
=======================

Build libril_mione and select /system/lib/libril_mione.so via rild.libpath.
The adapter loads the original /system/lib/libril-qc-qmi-1.so. It verifies the
blob's exported event-name table and version before using this contract.
No generic libril, telephony framework, or proprietary library is modified.

The shipped CAF table predates AOSP cell-info and initial-attach-APN requests.
AOSP 109..111 are unsupported; forwarding them unchanged would invoke CAF's
IMS-registration, IMS-SMS and data-profile handlers with the wrong payload.
Map the matching AOSP/CM requests 112..116 to CAF 109..113. Reject unsupported
requests without invoking the blob. Keep RIL v6 and all response layouts.

Unsolicited mappings (CAF -> CM11):
  1036 IMS network change -> 1037
  1038 void data network change -> 1002 registration refresh
  1039 supplementary-service result -> 1038
  1040 STK alpha -> 1039
  1041 UICC subscription status -> 1040
CAF tethered-mode/QoS/modify-call extensions have no matching framework event
and are dropped with a log. A nonempty CAF data-network-change payload violates
the verified contract and is rejected. Registration refresh makes CM11's
GsmServiceStateTracker poll both voice and data registration; it does not
fabricate a data-call list or supplementary-service result.

Tests use an isolated fake RIL under /data/local/tmp/mione-userspace. Build
libmione_fake_ril, libmione_ril_test, mione_ril_probe; stage them together there.
The probe checks request IDs, original tokens/pointers/lengths, callbacks,
unsupported requests, unsolicited mappings, and a callback during RIL_Init.
Passing this test and real modem initialization is not proof of calls/SMS/data
without a SIM. Recheck the numbered contract and ABI for a future Android/RIL
version; do not raise the blob's version to claim unsupported capabilities.
