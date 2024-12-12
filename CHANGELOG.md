# Changes since 2024-11-28

## Changes in Analysis

- [#13758](https://github.com/AliceO2Group/AliceO2/pull/13758) 2024-12-04: AOD: switch TracksQA from 000 to 001 by [@f3sch](https://github.com/f3sch)
- [#13765](https://github.com/AliceO2Group/AliceO2/pull/13765) 2024-12-10: [Run2,Converter] Add V0 on the fly table by [@mhemmer-cern](https://github.com/mhemmer-cern)
- [#13785](https://github.com/AliceO2Group/AliceO2/pull/13785) 2024-12-12: Bump trackextra table to the latest version by [@maciacco](https://github.com/maciacco)
- [#13787](https://github.com/AliceO2Group/AliceO2/pull/13787) 2024-12-12: O2-4592 - adding TOF dX and dZ to the track QA table with int8 precision by [@miranov25](https://github.com/miranov25)
## Changes in Common

- [#13762](https://github.com/AliceO2Group/AliceO2/pull/13762) 2024-12-03: Optionally select CTFs in timestamps or orbits range by [@shahor02](https://github.com/shahor02)
- [#13775](https://github.com/AliceO2Group/AliceO2/pull/13775) 2024-12-06: Fixes for MatchITSTPCQC + DCAFitter overrides bad covmat instead of throwing by [@shahor02](https://github.com/shahor02)
- [#13789](https://github.com/AliceO2Group/AliceO2/pull/13789) 2024-12-10: DCAFitterN: log-throttling for err.messages + user policy for bad CovMat by [@shahor02](https://github.com/shahor02)
## Changes in DataFormats

- [#13750](https://github.com/AliceO2Group/AliceO2/pull/13750) 2024-11-29: Updating hypernuclei information in O2 Databases by [@jditzelnew](https://github.com/jditzelnew)
## Changes in Detectors

- [#13749](https://github.com/AliceO2Group/AliceO2/pull/13749) 2024-11-29: TPC MC: Move hit exlusion after Track Ref creation by [@wiechula](https://github.com/wiechula)
- [#13737](https://github.com/AliceO2Group/AliceO2/pull/13737) 2024-12-03: ITS-GPU: Move Tracklet finder on GPU by [@mconcas](https://github.com/mconcas)
- [#13762](https://github.com/AliceO2Group/AliceO2/pull/13762) 2024-12-03: Optionally select CTFs in timestamps or orbits range by [@shahor02](https://github.com/shahor02)
- [#13758](https://github.com/AliceO2Group/AliceO2/pull/13758) 2024-12-04: AOD: switch TracksQA from 000 to 001 by [@f3sch](https://github.com/f3sch)
- [#13775](https://github.com/AliceO2Group/AliceO2/pull/13775) 2024-12-06: Fixes for MatchITSTPCQC + DCAFitter overrides bad covmat instead of throwing by [@shahor02](https://github.com/shahor02)
- [#13773](https://github.com/AliceO2Group/AliceO2/pull/13773) 2024-12-06: ITS: protect trackleting from exceeding maximum nROFs in weird situations by [@mconcas](https://github.com/mconcas)
- [#13777](https://github.com/AliceO2Group/AliceO2/pull/13777) 2024-12-08: AOD: Missing default init by [@f3sch](https://github.com/f3sch)
- [#13774](https://github.com/AliceO2Group/AliceO2/pull/13774) 2024-12-09: Do not compile tests which need GLFW if it's not found by [@ktf](https://github.com/ktf)
- [#13764](https://github.com/AliceO2Group/AliceO2/pull/13764) 2024-12-09: [FOCAL-81] Fixing the FOCAL-E pad geometry by [@hahassan7](https://github.com/hahassan7)
- [#13786](https://github.com/AliceO2Group/AliceO2/pull/13786) 2024-12-10: ITS tracking: Improve logging in case of OOM by [@mconcas](https://github.com/mconcas)
- [#13779](https://github.com/AliceO2Group/AliceO2/pull/13779) 2024-12-11: Add additional selections on ITS tracks by [@mpuccio](https://github.com/mpuccio)
- [#13795](https://github.com/AliceO2Group/AliceO2/pull/13795) 2024-12-11: Fix inverted check in rawtf-reader max-tf-per-file check by [@shahor02](https://github.com/shahor02)
- [#13785](https://github.com/AliceO2Group/AliceO2/pull/13785) 2024-12-12: Bump trackextra table to the latest version by [@maciacco](https://github.com/maciacco)
- [#13792](https://github.com/AliceO2Group/AliceO2/pull/13792) 2024-12-12: ITS tracking: Introduce configurable minimum pt per track length by [@mpuccio](https://github.com/mpuccio)
- [#13791](https://github.com/AliceO2Group/AliceO2/pull/13791) 2024-12-12: ITS-tracking::Async: optionally discard whole TF instead of breaking by [@mconcas](https://github.com/mconcas)
- [#13787](https://github.com/AliceO2Group/AliceO2/pull/13787) 2024-12-12: O2-4592 - adding TOF dX and dZ to the track QA table with int8 precision by [@miranov25](https://github.com/miranov25)
## Changes in Examples

- [#13788](https://github.com/AliceO2Group/AliceO2/pull/13788) 2024-12-12: HybridGenerator: Asyncronous + parallel event generation by [@sawenzel](https://github.com/sawenzel)
## Changes in Framework

- [#13759](https://github.com/AliceO2Group/AliceO2/pull/13759) 2024-12-02: DPL Analysis: improve arrow::Dataset support for TTree by [@ktf](https://github.com/ktf)
- [#13758](https://github.com/AliceO2Group/AliceO2/pull/13758) 2024-12-04: AOD: switch TracksQA from 000 to 001 by [@f3sch](https://github.com/f3sch)
- [#13771](https://github.com/AliceO2Group/AliceO2/pull/13771) 2024-12-05: DPL Analysis: fix for iterator not setting the originals correctly by [@aalkin](https://github.com/aalkin)
- [#13765](https://github.com/AliceO2Group/AliceO2/pull/13765) 2024-12-10: [Run2,Converter] Add V0 on the fly table by [@mhemmer-cern](https://github.com/mhemmer-cern)
- [#13793](https://github.com/AliceO2Group/AliceO2/pull/13793) 2024-12-11: DPL: extra signposts for InitTask callback by [@ktf](https://github.com/ktf)
- [#13790](https://github.com/AliceO2Group/AliceO2/pull/13790) 2024-12-11: Make sure only the signposts matching the provided string are enabled by [@ktf](https://github.com/ktf)
- [#13785](https://github.com/AliceO2Group/AliceO2/pull/13785) 2024-12-12: Bump trackextra table to the latest version by [@maciacco](https://github.com/maciacco)
- [#13796](https://github.com/AliceO2Group/AliceO2/pull/13796) 2024-12-12: Core: wrap internal symbols with anonymous namespace by [@mytkom](https://github.com/mytkom)
- [#13787](https://github.com/AliceO2Group/AliceO2/pull/13787) 2024-12-12: O2-4592 - adding TOF dX and dZ to the track QA table with int8 precision by [@miranov25](https://github.com/miranov25)
## Changes in Generators

- [#13745](https://github.com/AliceO2Group/AliceO2/pull/13745) 2024-12-05: Fraction based randomisation for Hybrid Gen by [@jackal1-66](https://github.com/jackal1-66)
- [#13766](https://github.com/AliceO2Group/AliceO2/pull/13766) 2024-12-05: New event generator/wrapper facilitating event-pools by [@sawenzel](https://github.com/sawenzel)
- [#13768](https://github.com/AliceO2Group/AliceO2/pull/13768) 2024-12-06: Reset mParticles before filled with new set of particles by [@pbuehler](https://github.com/pbuehler)
- [#13788](https://github.com/AliceO2Group/AliceO2/pull/13788) 2024-12-12: HybridGenerator: Asyncronous + parallel event generation by [@sawenzel](https://github.com/sawenzel)
## Changes in Steer

- [#13750](https://github.com/AliceO2Group/AliceO2/pull/13750) 2024-11-29: Updating hypernuclei information in O2 Databases by [@jditzelnew](https://github.com/jditzelnew)
## Changes in Utilities

- [#13782](https://github.com/AliceO2Group/AliceO2/pull/13782) 2024-12-09: QC-1253 Mergers: Shorter latency with multiple layers by [@knopers8](https://github.com/knopers8)
