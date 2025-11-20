/ Jenkinsfile
pipeline {
    agent any  // or 'docker', or a specific labeled node that has Docker
    environment {
        // Optional: reuse PlatformIO cache across builds by mounting a host dir/volume
        // Adjust this path or remove if you don't care about caching
        PIO_CACHE = "/tmp/pio"
    }
    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }
        stage('Build with PlatformIO (Docker)') {
            steps {
                script {
                    // Pull & run the same image you use in GitHub Actions
                    docker.image('ghcr.io/pbrier/platformio:latest').inside(
                        "-e HOME=${env.WORKSPACE} " +
                        "-w ${env.WORKSPACE} " +
                        "-v ${env.WORKSPACE}:${env.WORKSPACE} " +
                        "-v ${PIO_CACHE}:/tmp/pio"
                    ) {
                        sh 'platformio run'
                    }
                }
            }
        }
        stage('Archive firmware artifact') {
            steps {
                // Same artifact path as in your GitHub Action
                archiveArtifacts artifacts: '.pio/build/mkr_wifi1010/firmware.bin',
                                  fingerprint: true
            }
        }
    }
    post {
        always {
            // Optional: keep the workspace clean between builds
            cleanWs()
        }
    }
}
