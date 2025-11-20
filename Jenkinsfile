pipeline {
    agent any

    environment {
        // Optional: cache directory (similar to actions/cache)
        PIO_CACHE_DIR = "/tmp/pio-cache"
    }

    stages {
        stage('Checkout') {
            steps {
                checkout scm
            }
        }

        stage('Cache PlatformIO') {
            steps {
                cache(path: "${env.PIO_CACHE_DIR}", key: "pio-cache-${env.NODE_NAME}") {
                    echo "PlatformIO cache restored (or created)"
                }
            }
        }

        stage('Build with PlatformIO (Docker)') {
            steps {
                script {
                    // This reproduces the exact Docker command that the GitHub Action was running:
                    // docker run -e HOME=`pwd` --rm -it -w `pwd` -v `pwd`:`pwd` ghcr.io/pbrier/platformio:latest platformio run
                    docker.image('ghcr.io/pbrier/platformio:latest').inside("""
                        -e HOME=${env.WORKSPACE}
                        -w ${env.WORKSPACE}
                        -v ${env.WORKSPACE}:${env.WORKSPACE}
                        --rm
                    """) {
                        sh 'platformio run'
                    }
                }
            }
        }

        stage('Archive Firmware') {
            steps {
                archiveArtifacts artifacts: '.pio/build/mkr_wifi1010/firmware.bin', 
                                 fingerprint: true, 
                                 onlyIfSuccessful: true
            }
        }
    }

    post {
        always {
            cleanWs() // optional: clean workspace after build
        }
    }
}
