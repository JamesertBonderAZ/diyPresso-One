pipeline {
    agent any

    environment {
        PI_HOST = "gitlabrunner.local"
        PI_USER = "qteal"
        PI_PASS = "Welcome"
        FIRMWARE_PATH = ".pio/build/mkr_wifi1010/firmware.bin"
        REMOTE_PATH = "/tmp/firmware.bin"
    }

    stages {
        stage('Build diyPresso-One') {
            steps {
                checkout scm
                sh 'platformio run'
            }
        }

        stage('Send Firmware to Pi') {
            steps {
                sh """
                sshpass -p $PI_PASS scp $FIRMWARE_PATH $PI_USER@$PI_HOST:$REMOTE_PATH
                """
            }
        }

        stage('Flash Firmware on Pi') {
            steps {
                sh """
                sshpass -p $PI_PASS ssh $PI_USER@$PI_HOST 'DEVICE=$(readlink -f /dev/dut) && \
                echo "Uploading to diyPresso at $DEVICE" && \
                arduino-cli upload -b arduino:samd:mkrwifi1010 -p $DEVICE --input-file $REMOTE_PATH'
                """
            }
        }

        stage('Save Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/**/firmware.*', fingerprint: true
            }
        }
    }
}
