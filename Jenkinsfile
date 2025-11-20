pipeline {
    agent any

    environment {
        // You can define cache location if desired
        PIO_CACHE = "/tmp/pio"
    }

    stages {

        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Build firmware (PlatformIO Docker)') {
            steps {
                script {
                    docker.image('ghcr.io/pbrier/platformio:latest').inside(
                        "-e HOME=${WORKSPACE} -v ${WORKSPACE}:${WORKSPACE} -w ${WORKSPACE}"
                    ) {
                        sh 'platformio run'
                    }
                }
            }
        }

        stage('Archive firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/mkr_wifi1010/firmware.bin', fingerprint: true
            }
        }
    }
}
