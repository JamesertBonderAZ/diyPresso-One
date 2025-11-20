pipeline {
    agent any
    stages {
        stage('Build') {
            steps {
                checkout scm
                sh 'platformio run'
                // Save the firmware for later use on RPi agent
                stash name: 'firmware', includes: '.pio/build/mkr_wifi1010/firmware.bin'
            }
        }

        stage('Flash on RPi5') {
            agent { label 'rpi5' } 
            steps {
                // Bring the firmware blob to the Pi 5
                unstash 'firmware'
                sh '''
                    DEVICE=$(readlink -f /dev/dut || echo "/dev/ttyACM1")
                    echo "Using device: $DEVICE"

                    arduino-cli upload \
                        -b arduino:samd:mkrwifi1010 \
                        -p "$DEVICE" \
                        --input-file .pio/build/mkr_wifi1010/firmware.bin
                '''
            }
        }

        stage('Archive') {
            steps {
                archiveArtifacts artifacts: '.pio/build/**/firmware.*', fingerprint: true
            }
        }
    }
}
