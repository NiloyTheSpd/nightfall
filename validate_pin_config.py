#!/usr/bin/env python3
"""
Pin Configuration Validator for Project Nightfall
Validates pin assignments across all source files and checks for conflicts
"""

import re
import os
import sys
from typing import Dict, List, Set, Tuple

class PinValidator:
    def __init__(self):
        self.pin_definitions = {}
        self.file_pins = {}
        self.conflicts = []
        self.warnings = []
        
    def load_pin_definitions(self, pins_file: str) -> bool:
        """Load pin definitions from pins.h"""
        try:
            with open(pins_file, 'r') as f:
                content = f.read()
                
            # Extract #define PIN_XXX patterns
            pin_pattern = r'#define\s+(PIN_\w+)\s+(\d+)'
            matches = re.findall(pin_pattern, content)
            
            for pin_name, pin_number in matches:
                self.pin_definitions[pin_name] = int(pin_number)
                
            print(f"✅ Loaded {len(self.pin_definitions)} pin definitions from {pins_file}")
            return True
            
        except FileNotFoundError:
            print(f"❌ Error: {pins_file} not found")
            return False
        except Exception as e:
            print(f"❌ Error loading pin definitions: {e}")
            return False
    
    def scan_source_files(self, src_dir: str) -> bool:
        """Scan source files for pin usage"""
        try:
            cpp_extensions = ['.cpp', '.h', '.hpp']
            
            for root, dirs, files in os.walk(src_dir):
                for file in files:
                    if any(file.endswith(ext) for ext in cpp_extensions):
                        file_path = os.path.join(root, file)
                        self.scan_file(file_path)
                        
            print(f"✅ Scanned {len(self.file_pins)} source files")
            return True
            
        except Exception as e:
            print(f"❌ Error scanning source files: {e}")
            return False
    
    def scan_file(self, file_path: str):
        """Scan individual file for pin usage"""
        try:
            with open(file_path, 'r') as f:
                content = f.read()
                
            # Find pin usage patterns
            pin_usage_patterns = [
                r'PIN_\w+',  # Direct pin definitions
                r'GPIO\s*(\d+)',  # GPIO pin references
                r'pinMode\s*\(\s*(\d+)',  # pinMode calls
                r'digitalWrite\s*\(\s*(\d+)',  # digitalWrite calls
                r'analogRead\s*\(\s*(\d+)',  # analogRead calls
            ]
            
            file_pins = set()
            for pattern in pin_usage_patterns:
                matches = re.findall(pattern, content)
                for match in matches:
                    if match.isdigit():
                        file_pins.add(int(match))
            
            if file_pins:
                self.file_pins[file_path] = file_pins
                
        except Exception as e:
            print(f"⚠️ Warning: Could not scan {file_path}: {e}")
    
    def validate_pin_assignments(self) -> bool:
        """Validate pin assignments for conflicts and safety"""
        all_used_pins = set()
        
        # Collect all pins used in source files
        for file_pins in self.file_pins.values():
            all_used_pins.update(file_pins)
        
        # Check for pin conflicts
        board_pins = {
            'rear': set([13, 14, 18, 19, 21, 22, 23, 25, 26, 27, 32, 33, 36, 4]),
            'front': set([13, 14, 18, 19, 21, 22, 23, 25, 26, 27]),
            'camera': set([33])
        }
        
        # Check for unsafe pin usage
        unsafe_pins = {
            'rear': set([0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 20, 34, 35, 37, 38, 39]),
            'front': set([0, 1, 2, 3, 5, 6, 7, 8, 9, 10, 11, 12, 15, 16, 17, 20, 32, 33, 34, 35, 36, 37, 38, 39]),
            'camera': set([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 34, 35, 36, 37, 38, 39])
        }
        
        conflicts_found = False
        
        # Check for unsafe pin usage
        for pin in all_used_pins:
            if pin in unsafe_pins['rear'] or pin in unsafe_pins['front'] or pin in unsafe_pins['camera']:
                self.warnings.append(f"⚠️ Warning: Pin GPIO{pin} may be unsafe for external use")
                conflicts_found = True
        
        # Check for critical voltage divider requirements
        voltage_critical_pins = [36]  # HC-SR04 Echo pin
        for pin in all_used_pins:
            if pin in voltage_critical_pins:
                self.warnings.append(f"🔴 CRITICAL: GPIO{pin} requires 5V→3.3V voltage divider!")
                conflicts_found = True
        
        return not conflicts_found
    
    def check_uart_connections(self) -> bool:
        """Check UART cross-connection specifications"""
        expected_connections = [
            ('rear_tx', 22, 'front_rx', 22),
            ('rear_rx', 21, 'front_tx', 23)
        ]
        
        for conn_name, rear_pin, front_pin, expected_front_pin in expected_connections:
            if front_pin != expected_front_pin:
                self.conflicts.append(f"❌ UART {conn_name}: Rear GPIO{rear_pin} → Front GPIO{front_pin} (expected GPIO{expected_front_pin})")
                return False
        
        print("✅ UART cross-connections verified")
        return True
    
    def check_motor_assignments(self) -> bool:
        """Check motor pin assignments"""
        # Expected motor pins by board
        expected_motor_pins = {
            'rear': [13, 14, 18, 19, 23, 27],
            'front': [13, 14, 18, 19, 21, 23, 25, 26, 27]
        }
        
        motor_pins_found = {
            'rear': [],
            'front': []
        }
        
        # Scan for motor-related pin usage
        for file_path, pins in self.file_pins.items():
            if 'rear' in file_path.lower():
                motor_pins_found['rear'].extend([p for p in pins if p in expected_motor_pins['rear']])
            elif 'front' in file_path.lower():
                motor_pins_found['front'].extend([p for p in pins if p in expected_motor_pins['front']])
        
        # Check for missing motor pins
        for board, expected_pins in expected_motor_pins.items():
            found_pins = set(motor_pins_found[board])
            missing_pins = set(expected_pins) - found_pins
            
            if missing_pins:
                self.warnings.append(f"⚠️ Warning: {board.title()} ESP32 missing motor pins: {missing_pins}")
        
        return True
    
    def generate_report(self) -> str:
        """Generate validation report"""
        report = []
        report.append("=" * 60)
        report.append("PROJECT NIGHTFALL PIN CONFIGURATION VALIDATION REPORT")
        report.append("=" * 60)
        report.append("")
        
        # Summary statistics
        report.append("📊 VALIDATION SUMMARY:")
        report.append(f"  • Pin definitions loaded: {len(self.pin_definitions)}")
        report.append(f"  • Source files scanned: {len(self.file_pins)}")
        report.append(f"  • Total unique pins used: {len(set().union(*self.file_pins.values()))}")
        report.append("")
        
        # File scan results
        if self.file_pins:
            report.append("📁 FILES SCANNED:")
            for file_path, pins in self.file_pins.items():
                report.append(f"  • {file_path}: GPIO{{{', '.join(map(str, sorted(pins)))}}}")
            report.append("")
        
        # Pin definitions
        if self.pin_definitions:
            report.append("📌 PIN DEFINITIONS:")
            for pin_name, pin_number in sorted(self.pin_definitions.items()):
                report.append(f"  • {pin_name}: GPIO{pin_number}")
            report.append("")
        
        # Warnings and conflicts
        if self.warnings:
            report.append("⚠️ WARNINGS:")
            for warning in self.warnings:
                report.append(f"  {warning}")
            report.append("")
        
        if self.conflicts:
            report.append("❌ CONFLICTS:")
            for conflict in self.conflicts:
                report.append(f"  {conflict}")
            report.append("")
        
        # Safety recommendations
        report.append("🔒 SAFETY RECOMMENDATIONS:")
        report.append("  • Always use voltage divider on HC-SR04 Echo (GPIO36)")
        report.append("  • ESP32-CAM only uses GPIO33 for external connections")
        report.append("  • Verify 3.3V current capacity for all sensors")
        report.append("  • Test UART connections before deployment")
        report.append("")
        
        # Validation status
        if not self.conflicts and not self.warnings:
            report.append("✅ VALIDATION PASSED: No conflicts or warnings found!")
        elif not self.conflicts:
            report.append("⚠️ VALIDATION PASSED WITH WARNINGS: Review warnings above")
        else:
            report.append("❌ VALIDATION FAILED: Conflicts must be resolved")
        
        report.append("")
        report.append("=" * 60)
        
        return "\n".join(report)

def main():
    validator = PinValidator()
    
    # Paths
    pins_file = "include/pins.h"
    src_dir = "src"
    
    print("🔍 Starting Project Nightfall Pin Configuration Validation...")
    print()
    
    # Load pin definitions
    if not validator.load_pin_definitions(pins_file):
        sys.exit(1)
    
    # Scan source files
    if not validator.scan_source_files(src_dir):
        print("⚠️ Warning: Some files could not be scanned")
    
    # Run validations
    validator.validate_pin_assignments()
    validator.check_uart_connections()
    validator.check_motor_assignments()
    
    # Generate and display report
    report = validator.generate_report()
    print(report)
    
    # Save report to file
    with open("pin_validation_report.txt", "w") as f:
        f.write(report)
    
    print("📄 Validation report saved to pin_validation_report.txt")
    
    # Exit with appropriate code
    if validator.conflicts:
        sys.exit(1)  # Exit with error code if conflicts found
    else:
        sys.exit(0)  # Exit successfully

if __name__ == "__main__":
    main()