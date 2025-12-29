# Project Documentation Rules (Non-Obvious Only)

**Primary Documentation Source:** `docs/rescue_robot_docs.md` contains the complete implementation reference - NOT inline code comments

**Agent Protocol Rules:** `rules/agent-protocol.md` contains mandatory MCP tool usage workflow for AI agents working in this repository

**Documentation Pattern:** Library implementations and examples are in `docs/` directory, not in standard code examples folders

**WiFi Configuration:** Network credentials hardcoded as "ProjectNightfall"/"rescue2025" in multiple files, not centralized in config

**Multi-Board Context:** Documentation covers three separate ESP32 systems (Master/Slave/Camera) with different upload procedures and monitoring requirements

**Implementation References:** Hardware assembly, testing procedures, and troubleshooting guides are comprehensive in `docs/` - essential for understanding project scope

**Code Structure:** Despite Arduino framework, project uses modular C++ class structure in `lib/` directory with separation of concerns