#!/usr/bin/env python3
"""
PlatformIO Configuration Validator
Validates the platformio.ini file for common configuration issues
"""

import configparser
import re
import sys
from pathlib import Path

def validate_platformio_config(config_file):
    """Validate PlatformIO configuration file"""
    print(f"Validating {config_file}...")
    
    issues = []
    warnings = []
    
    try:
        config = configparser.ConfigParser()
        config.read(config_file)
        
        # Check 1: build_flags in [platformio] section (should be in [env])
        if 'platformio' in config:
            if 'build_flags' in config['platformio']:
                issues.append("ERROR: build_flags found in [platformio] section - move to [env] section")
        
        # Check 2: monitor_flags in any section (unsupported)
        for section_name in config.sections():
            section = config[section_name]
            if 'monitor_flags' in section:
                issues.append(f"ERROR: monitor_flags found in [{section_name}] - unsupported option")
        
        # Check 3: AsyncTCP version consistency
        asynctcp_versions = {}
        for section_name in config.sections():
            section = config[section_name]
            if 'lib_deps' in section:
                lib_deps = section['lib_deps']
                if 'AsyncTCP' in lib_deps:
                    # Extract version using regex
                    version_match = re.search(r'AsyncTCP@(.+)', lib_deps)
                    if version_match:
                        version = version_match.group(1)
                        asynctcp_versions[section_name] = version
        
        if len(set(asynctcp_versions.values())) > 1:
            warnings.append(f"WARNING: AsyncTCP versions inconsistent: {asynctcp_versions}")
        
        # Check 4: Valid section names
        for section_name in config.sections():
            if not section_name.startswith('env:'):
                if section_name not in ['platformio', 'env']:
                    warnings.append(f"WARNING: Non-standard section name: [{section_name}]")
        
        # Check 5: Required sections exist
        required_sections = ['platformio', 'env']
        for required in required_sections:
            if required not in config.sections():
                issues.append(f"ERROR: Missing required section: [{required}]")
        
        # Check 6: build_src_filter validation
        for section_name in config.sections():
            if section_name.startswith('env:'):
                section = config[section_name]
                if 'build_src_filter' in section:
                    src_filter = section['build_src_filter']
                    if not re.search(r'\+\<.*\.cpp\>', src_filter):
                        warnings.append(f"WARNING: Unusual build_src_filter in [{section_name}]: {src_filter}")
        
        return issues, warnings
        
    except Exception as e:
        return [f"ERROR: Configuration parse error: {str(e)}"], []

def main():
    config_file = "platformio.ini"
    
    if not Path(config_file).exists():
        print(f"ERROR: Configuration file not found: {config_file}")
        sys.exit(1)
    
    issues, warnings = validate_platformio_config(config_file)
    
    print("\n" + "="*60)
    print("PLATFORMIO CONFIGURATION VALIDATION RESULTS")
    print("="*60)
    
    if not issues and not warnings:
        print("SUCCESS: Configuration appears to be valid!")
    else:
        if issues:
            print("\nCRITICAL ISSUES (must fix):")
            for issue in issues:
                print(f"  {issue}")
        
        if warnings:
            print("\nWARNINGS (recommended fixes):")
            for warning in warnings:
                print(f"  {warning}")
    
    print("\n" + "="*60)
    
    if issues:
        print("ERROR: Configuration validation FAILED")
        sys.exit(1)
    elif warnings:
        print("WARNING: Configuration validation PASSED with warnings")
        sys.exit(0)
    else:
        print("SUCCESS: Configuration validation PASSED")
        sys.exit(0)

if __name__ == "__main__":
    main()