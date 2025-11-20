
pipeline {
    agent any

    stages {
        stage('Build diyPresso-One') {
            steps {
                checkout scm
                sh 'platformio run'
            }
        }

        stage('Archive Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/**/firmware.*', fingerprint: true
            }
        }

        // THIS IS THE NEW MAGIC STAGE
        stage('Flash to Arduino via Raspberry Pi') {
            when {
                // Only flash on main branch (or remove this line to flash on every branch)
                branch 'main'
            }
            steps {
                script {
                    // Copy the firmware to the Pi and flash it
                    sh '''
                        # Copy firmware to Raspberry Pi
                        scp .pio/build/mkr_wifi1010/firmware.bin qteal@gitlabrunner.local:/home/qteal/firmware.bin

                        # SSH and flash it (arduino-cli must be installed on the Pi)
                        ssh qteal@gitlabrunner.local << 'EOF'
                            echo "123" | sudo -S echo "Starting flash..."  # if sudo needs password
                            DEVICE=$(readlink -f /dev/dut 2>/dev/null || ls /dev/ttyACM* | head -1)
                            echo "Uploading to diyPresso at $DEVICE"
                            arduino-cli upload -b arduino:samd:mkrwifi1010 -p "$DEVICE" --input-file /home/qteal/firmware.bin || echo "Upload failed – is board connected?"
                            rm -f /home/qteal/firmware.bin
                        EOF
                    '''
                }
            }
        }
    }

    post {
        always {
            cleanWs()
        }
    }
}
