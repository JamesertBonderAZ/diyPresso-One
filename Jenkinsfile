pipeline {
    agent any   // Build runs on the main Jenkins node (Windows Docker)

    stages {
        stage('Build') {
            steps {
                checkout scm
                sh 'platformio run'
            }
        }

        stage('Flash via Raspberry Pi') {
            agent { label 'rpi5' }  // <--- IMPORTANT: run this stage on the Pi
            steps {
                sh '''
                    DEVICE=$(readlink -f /dev/dut || echo "/dev/ttyACM0")

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
                archiveArtifacts '.pio/build/**/firmware.*'
            }
        }
    }
}
