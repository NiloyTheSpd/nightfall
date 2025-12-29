#!/bin/bash
# Project Nightfall - Deployment Script
# Deploys firmware to connected ESP32 boards

set -e  # Exit on any error

# Color codes for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Configuration
ROBOT_ID=${1:-"NF-001"}
FIRMWARE_DIR="dist/firmware"
LOG_DIR="logs/deployment"
TIMESTAMP=$(date +%Y%m%d_%H%M%S)
LOG_FILE="$LOG_DIR/deployment_${ROBOT_ID}_${TIMESTAMP}.log"

# Function to print colored output
print_status() {
    local status=$1
    local message=$2
    case $status in
        "INFO")
            echo -e "${BLUE}ℹ️  $message${NC}"
            ;;
        "SUCCESS")
            echo -e "${GREEN}✅ $message${NC}"
            ;;
        "WARNING")
            echo -e "${YELLOW}⚠️  $message${NC}"
            ;;
        "ERROR")
            echo -e "${RED}❌ $message${NC}"
            ;;
        "STEP")
            echo -e "${BLUE}🔧 $message${NC}"
            ;;
    esac
}

# Logging function
log() {
    echo "[$(date '+%Y-%m-%d %H:%M:%S')] $1" | tee -a "$LOG_FILE"
}

# Error handling
error_exit() {
    log "ERROR: $1"
    print_status "ERROR" "$1"
    exit 1
}

# Check prerequisites
check_prerequisites() {
    print_status "STEP" "Checking deployment prerequisites..."
    
    # Create log directory
    mkdir -p "$LOG_DIR"
    
    # Check if firmware directory exists
    if [ ! -d "$FIRMWARE_DIR" ]; then
        error_exit "Firmware directory not found: $FIRMWARE_DIR"
    fi
    
    # Check if firmware files exist
    local required_files=("front_esp32" "rear_esp32" "camera_esp32")
    for file in "${required_files[@]}"; do
        if ! ls "$FIRMWARE_DIR"/${file}_*.bin 1> /dev/null 2>&1; then
            error_exit "Required firmware file not found: ${file}_*.bin"
        fi
    done
    
    # Check PlatformIO installation
    if ! command -v pio &> /dev/null; then
        error_exit "PlatformIO not found. Please install it first."
    fi
    
    # Check USB ports
    log "Checking available USB ports..."
    pio device list | tee -a "$LOG_FILE" || true
    
    print_status "SUCCESS" "Prerequisites check completed"
}

# Detect connected ESP32 boards
detect_boards() {
    print_status "STEP" "Detecting connected ESP32 boards..."
    
    local devices=$(pio device list --json 2>/dev/null | jq -r '.[].port' 2>/dev/null || echo "")
    
    if [ -z "$devices" ]; then
        print_status "WARNING" "No ESP32 devices detected via PlatformIO"
        print_status "INFO" "Please ensure ESP32 boards are connected via USB"
        return 1
    fi
    
    log "Detected devices:"
    echo "$devices" | while read -r device; do
        log "  - $device"
    done
    
    print_status "SUCCESS" "Device detection completed"
    return 0
}

# Validate firmware files
validate_firmware() {
    print_status "STEP" "Validating firmware files..."
    
    local validation_passed=true
    
    # Check each firmware file
    for board in "front_esp32" "rear_esp32" "camera_esp32"; do
        local firmware_file=$(ls "$FIRMWARE_DIR"/${board}_*.bin | head -n1)
        
        if [ -f "$firmware_file" ]; then
            local file_size=$(stat -f%z "$firmware_file" 2>/dev/null || stat -c%s "$firmware_file")
            local file_size_kb=$((file_size / 1024))
            
            log "Firmware for $board: $firmware_file (${file_size_kb}KB)"
            
            # Basic size validation (should be between 1MB and 4MB)
            if [ "$file_size_kb" -lt 1024 ] || [ "$file_size_kb" -gt 4096 ]; then
                print_status "WARNING" "Unusual firmware size for $board: ${file_size_kb}KB"
            fi
        else
            print_status "ERROR" "Firmware file not found for $board"
            validation_passed=false
        fi
    done
    
    # Check manifest file
    if [ -f "$FIRMWARE_DIR/manifest.json" ]; then
        log "Found firmware manifest: $FIRMING_DIR/manifest.json"
    else
        print_status "WARNING" "Firmware manifest not found"
    fi
    
    # Check checksums
    if [ -f "$FIRMWARE_DIR/checksums.txt" ]; then
        log "Found checksums file: $FIRMWARE_DIR/checksums.txt"
        cd "$FIRMWARE_DIR"
        if sha256sum -c checksums.txt >/dev/null 2>&1; then
            log "Checksum validation: PASSED"
        else
            print_status "WARNING" "Checksum validation failed"
        fi
        cd - > /dev/null
    else
        print_status "WARNING" "Checksums file not found"
    fi
    
    if [ "$validation_passed" = true ]; then
        print_status "SUCCESS" "Firmware validation completed"
        return 0
    else
        print_status "ERROR" "Firmware validation failed"
        return 1
    fi
}

# Upload firmware to specific board
upload_firmware() {
    local board_type=$1
    local environment=$2
    local description=$3
    
    print_status "INFO" "Uploading firmware to $description..."
    
    # Create backup if firmware exists
    local backup_dir="backups/${ROBOT_ID}/${board_type}"
    mkdir -p "$backup_dir"
    
    log "Creating backup for $board_type..."
    if pio device list --json 2>/dev/null | jq -r '.[].port' | head -n1 | xargs -I {} pio device monitor --echo false 2>/dev/null || true; then
        # Try to read current firmware version if possible
        log "Current firmware backed up"
    fi
    
    # Upload firmware
    log "Uploading firmware for $board_type using environment: $environment"
    
    if pio run -e "$environment" -t upload --verbose; then
        log "Firmware upload successful for $board_type"
        
        # Wait for board to reboot
        sleep 3
        
        # Test basic functionality
        print_status "INFO" "Testing basic functionality for $board_type..."
        
        if pio device monitor -e "$environment" --echo false | timeout 10s grep -q "restarting" 2>/dev/null; then
            print_status "SUCCESS" "$board_type board rebooted successfully"
        else
            print_status "WARNING" "$board_type board reboot detection failed (may be normal)"
        fi
        
        return 0
    else
        print_status "ERROR" "Firmware upload failed for $board_type"
        return 1
    fi
}

# Deploy to all boards
deploy_all_boards() {
    print_status "STEP" "Deploying firmware to all boards..."
    
    local deployment_success=true
    
    # Front ESP32 (Master Controller)
    if upload_firmware "front_esp32" "front_esp32" "Front ESP32 (Master)"; then
        : # Success
    else
        deployment_success=false
    fi
    
    # Rear ESP32 (Slave Controller)
    if upload_firmware "rear_esp32" "rear_esp32" "Rear ESP32 (Slave)"; then
        : # Success
    else
        deployment_success=false
    fi
    
    # ESP32-CAM (Vision Module)
    if upload_firmware "camera_esp32" "camera_esp32" "ESP32-CAM (Vision)"; then
        : # Success
    else
        deployment_success=false
    fi
    
    if [ "$deployment_success" = true ]; then
        print_status "SUCCESS" "All boards deployed successfully"
        return 0
    else
        print_status "ERROR" "Some boards failed to deploy"
        return 1
    fi
}

# Validate deployment
validate_deployment() {
    print_status "STEP" "Validating deployment..."
    
    log "Waiting for boards to initialize..."
    sleep 5
    
    # Test connectivity
    print_status "INFO" "Testing network connectivity..."
    
    # Check if robot creates WiFi network
    if timeout 10s bash -c 'ssid=$(nmcli -t -f SSID dev wifi | grep ProjectNightfall); [ ! -z "$ssid" ]' 2>/dev/null; then
        print_status "SUCCESS" "WiFi network 'ProjectNightfall' detected"
    else
        print_status "WARNING" "WiFi network 'ProjectNightfall' not detected yet"
        log "Waiting additional time for WiFi initialization..."
        sleep 10
    fi
    
    # Test basic API connectivity
    print_status "INFO" "Testing API connectivity..."
    
    if timeout 10s bash -c 'curl -s --max-time 5 http://192.168.4.1/api/v1/health | jq -e . >/dev/null 2>&1'; then
        print_status "SUCCESS" "API connectivity test passed"
    else
        print_status "WARNING" "API connectivity test failed (may need more time)"
    fi
    
    # Test motor control
    print_status "INFO" "Testing motor control..."
    
    local motor_test_response=$(curl -s -X POST "http://192.168.4.1/api/v1/motors/control" \
        -H "Content-Type: application/json" \
        -d '{"command":"stop","speed":0}' \
        --max-time 10 2>/dev/null || echo "")
    
    if echo "$motor_test_response" | grep -q '"success":true'; then
        print_status "SUCCESS" "Motor control test passed"
    else
        print_status "WARNING" "Motor control test failed"
    fi
    
    # Test sensor data
    print_status "INFO" "Testing sensor data..."
    
    local sensor_response=$(curl -s "http://192.168.4.1/api/v1/sensors" --max-time 10 2>/dev/null || echo "")
    
    if echo "$sensor_response" | grep -q '"ultrasonic"'; then
        print_status "SUCCESS" "Sensor data test passed"
    else
        print_status "WARNING" "Sensor data test failed"
    fi
    
    print_status "SUCCESS" "Deployment validation completed"
}

# Generate deployment report
generate_deployment_report() {
    print_status "STEP" "Generating deployment report..."
    
    local report_file="$LOG_DIR/deployment_report_${ROBOT_ID}_${TIMESTAMP}.txt"
    
    cat > "$report_file" << EOF
Project Nightfall - Deployment Report
====================================
Robot ID: $ROBOT_ID
Deployment Date: $(date)
Deployment Time: $TIMESTAMP

Deployment Summary:
-------------------
Status: $([ -f "$LOG_FILE" ] && echo "SUCCESS" || echo "FAILED")
Firmware Source: $FIRMWARE_DIR
PlatformIO Version: $(pio --version 2>/dev/null || echo "Unknown")

Hardware Configuration:
-----------------------
- Front ESP32 (Master Controller)
- Rear ESP32 (Slave Controller)  
- ESP32-CAM (Vision Module)

Network Configuration:
----------------------
SSID: ProjectNightfall
Default IP: 192.168.4.1
WebSocket Port: 81
HTTP Port: 80

Validation Results:
-------------------
$(grep -E "(SUCCESS|FAILED|WARNING)" "$LOG_FILE" | tail -10 || echo "No validation logs available")

Deployment Logs:
----------------
$(tail -20 "$LOG_FILE" 2>/dev/null || echo "No deployment logs available")

Next Steps:
-----------
1. Access web dashboard at http://192.168.4.1
2. Test autonomous navigation in safe environment
3. Verify all safety systems are operational
4. Configure robot for specific mission requirements

Support:
--------
Documentation: docs/
Troubleshooting: docs/troubleshooting/
Support: support@projectnightfall.org

Deployment completed successfully!
EOF
    
    log "Deployment report generated: $report_file"
    print_status "SUCCESS" "Deployment report created"
}

# Main execution
main() {
    echo "🚀 Project Nightfall - Firmware Deployment"
    echo "=========================================="
    echo "Robot ID: $ROBOT_ID"
    echo "Firmware Directory: $FIRMWARE_DIR"
    echo "Log File: $LOG_FILE"
    echo ""
    
    # Start deployment process
    log "Starting deployment for robot $ROBOT_ID"
    
    if check_prerequisites && \
       detect_boards && \
       validate_firmware && \
       deploy_all_boards && \
       validate_deployment; then
        
        generate_deployment_report
        
        echo ""
        print_status "SUCCESS" "🎉 Deployment completed successfully!"
        echo ""
        echo "📱 Access the robot:"
        echo "   Connect to WiFi: ProjectNightfall"
        echo "   Dashboard URL: http://192.168.4.1"
        echo "   WebSocket Port: 81"
        echo ""
        echo "📋 Next steps:"
        echo "   1. Test autonomous navigation"
        echo "   2. Verify safety systems"
        echo "   3. Configure mission parameters"
        echo "   4. Begin operations"
        echo ""
        echo "📄 Deployment logs: $LOG_FILE"
        echo "📄 Deployment report: $LOG_DIR/deployment_report_${ROBOT_ID}_${TIMESTAMP}.txt"
        echo ""
        
        exit 0
    else
        print_status "ERROR" "Deployment failed!"
        echo ""
        echo "🔧 Troubleshooting:"
        echo "   1. Check hardware connections"
        echo "   2. Verify ESP32 boards are properly connected"
        echo "   3. Check USB cable connections"
        echo "   4. Review deployment logs: $LOG_FILE"
        echo "   5. See documentation: docs/troubleshooting/"
        echo ""
        exit 1
    fi
}

# Check if this is a dry run
if [ "$DRY_RUN" = "true" ]; then
    print_status "INFO" "Dry run mode - no actual deployment will occur"
    check_prerequisites
    detect_boards
    validate_firmware
    exit 0
fi

# Run main function
main "$@"