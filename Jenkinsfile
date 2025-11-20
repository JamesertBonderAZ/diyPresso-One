pipeline {
    agent { label 'rpi5' }

    stages {
        stage('Build') {
            steps {
                checkout scm
                sh 'platformio run'
            }
        }

        stage('Flash') {
            steps {
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
                archiveArtifacts '.pio/build/**/firmware.*'
            }
        }
    }
}
