# Distribution gate

NERON STREAM PROJECT source is GPL-2.0-or-later, consistent with libobs.
OBS source: https://github.com/obsproject/obs-studio/tree/31.0.3
Pinned commit: fcd1910bf5116b69404a6ecdda6efedd1d00ebdf.

Do not publicly distribute the development packages until:
- Full GPL license text and copyright notices accompany NERON STREAM PROJECT and OBS.
- Corresponding source for the exact binaries and build modifications is supplied.
- Qt's exact bundled version, license texts and dynamic-linking/relinking obligations are audited.
- FFmpeg configuration, component licenses and corresponding source are collected.
- Every bundled DLL/plugin is inventoried with license, source and redistribution rights.
- NVIDIA SDK notices are checked; never bundle the NVIDIA driver.
- MSVC runtime deployment is validated on a clean machine.

The build currently stages OBS-installed dependencies; it is not yet a minimal,
fully license-audited distribution. This file is a checklist, not legal clearance.
