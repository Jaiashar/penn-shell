# Source provenance

Historical Penn CIS 548 team project, Spring 2021.

Source: `CIS548/21sp-project-1-group-81`

Revision: `6c363ce06785b92ef64d9c5d6d00d49bf61a8fdf` (`HEAD`).

This publication began as a source snapshot with new documentation and a Linux build container. Contributor notices are preserved. The parser object is a supplied Linux x86-64 course dependency; its source was not included in the original repository. No new license is granted for course-provided or third-party material.

## Subsequent maintenance

October 11, 2026: added automated build and interactive smoke tests. The append-redirection check exposed a historical bug: `>>` opened its destination with `O_CREAT | O_APPEND` but omitted `O_WRONLY`, causing a bad-file-descriptor write failure. The public implementation now includes write permission for both append and overwrite modes. The test checks the actual file bytes after both operations.
